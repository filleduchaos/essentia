/*
 * Copyright (C) 2006-2010 Music Technology Group (MTG)
 *                         Universitat Pompeu Fabra
 *
 */

#ifndef ESSENTIA_STREAMINGALGORITHM_H
#define ESSENTIA_STREAMINGALGORITHM_H

#include "configurable.h"
#include "sourcebase.h"
#include "sinkbase.h"
#include "threading.h"

namespace essentia {
namespace scheduler {

  class Network;

} // namespace scheduler
} // namespace essentia

namespace essentia {
namespace streaming {


class ProcessTask;


/**
 * Global function used for connecting algorithms.
 */
void connect(SourceBase& source, SinkBase& sink);

inline void operator>>(SourceBase& source, SinkBase& sink) {
  connect(source, sink);
}

/**
 * Global function used for connecting algorithms. Same function as the
 * previous one, but with the order of arguments reversed.
 */
inline void connect(SinkBase& sink, SourceBase& source) {
  connect(source, sink);
}

/**
 * Global function used for disconnecting outputs from inputs.
 */
void disconnect(SourceBase& source, SinkBase& sink);

/**
 * Global function used for connecting algorithms. API is not as nice as the
 * other one, but error messages in case a connection fails are more explicit.
 */
void connect(Algorithm* sourceAlgo, const std::string& sourcePort,
             Algorithm* sinkAlgo, const std::string& sinkPort);



/**
 * This is the return type of the Algorithm::acquireData() and
 * Algorithm::process() methods.
 *
 * It can have 4 different values:
 *
 *  - OK        means that all sources and sinks could acquire enough tokens.
 *              when you return this from the process() method it means you
 *              actually managed to consume and/or produce something.
 *
 *  - CONTINUE  synonym with OK, means we did do something and want to continue
 *              consuming/producing.
 *
 *  - PASS      means that you don't want to do anything at the moment, for
 *              instance if you are waiting for the end of the stream. This
 *              can only be returned by the process() method.
 *              (implementation detail: in effect, this means the same as
 *              NO_INPUT, which is that everything that could be consumed has
 *              been consumed, but it looks semantically better in certain
 *              places, hence its existence)
 *
 *  - FINISHED  means that you returned something, but that you don't want
 *              to be called anymore. This can only be returned by the
 *              process() method, and should be used by those algorithms
 *              that wait for the end of the stream to output a value.
 *
 *  - NO_INPUT  means that there was at least one Sink for which there were
 *              not enough tokens available.
 *
 *  - NO_OUTPUT means that there was at least one Source for which there were
 *              not enough tokens available (output buffer full).
 */
enum AlgorithmStatus {
  OK = 0,
  CONTINUE = 0,
  PASS,
  FINISHED,
  NO_INPUT,
  NO_OUTPUT
};

/**
 * A streaming::Algorithm is an algorithm that can be used in streaming mode.
 * It is very similar to a 'classic' algorithm, but instead of having inputs
 * and outputs you need to set manually each time you want to call the
 * process() method, they have input stream connectors (called Sinks, because
 * data flows into them as water in a sink) and output stream connectors (called
 * Sources, because data flows out of them).
 *
 * Connections between two algorithms (actually, between a source and a sink)
 * represent a flow of Tokens, which can be of any type, as long as the Source
 * and Sink are themselves of the same type. A token is the smallest unit you
 * can have in your stream. For example, for an audio stream, the token type
 * will be Real, because a single audio sample is represented as a Real. A
 * frame cutter which outputs frames will have the Token type set to
 * vector<Real>, because you can't take smaller than that, otherwise the frame
 * would be incomplete. A framecutter is then a strange beast, because it has
 * a Sink<Real> (input is a stream of audio samples), but has also a
 * Source<vector<Real> >, because it outputs whole frames one by one. This
 * might seem strange, but is not a problem at all.
 *
 * NB: of course, the input of the framecutter will consume tokens much more
 *     quickly than the rate at which it produces output tokens (frames). That
 *     is not a problem at all.
 *
 */
class ESSENTIA_API Algorithm : public Configurable {

 public:
  static const std::string processingMode;

  typedef OrderedMap<SinkBase> InputMap;
  typedef OrderedMap<SourceBase> OutputMap;

  DescriptionMap inputDescription;
  DescriptionMap outputDescription;


 public:

  Algorithm() : _shouldStop(false)
#ifdef DEBUGGING_ENABLED
      , nProcess(0)
#endif
      {}

  virtual ~Algorithm() {
    // Note: no need to do anything here wrt to sources and sinks, as they
    //       disconnect themselves when destroyed.
  }


  SinkBase& input(const std::string& name);
  SourceBase& output(const std::string& name);

  SinkBase& input(int idx);
  SourceBase& output(int idx);

  const InputMap& inputs() const { return _inputs; }
  const OutputMap& outputs() const { return _outputs; }


  /**
   * Returns the names of all the inputs that have been defined for this algorithm.
   */
  std::vector<std::string> inputNames() const { return _inputs.keys(); }

  /**
   * Returns the names of all the outputs that have been defined for this algorithm.
   */
  std::vector<std::string> outputNames() const { return _outputs.keys(); }


  /**
   * Sets whether an algorithm should stop as soon as it has finished processing
   * all of its inputs. This is most often called when the algorithm has received
   * an STOP_WHEN_DONE signal (at the end of the stream).
   */
  virtual void shouldStop(bool stop);

  /**
   * Returns whether the algorithm should stop, ie: it has received an
   * end-of-stream signal.
   */
  virtual bool shouldStop() const { return _shouldStop; }



  /**
   * Disconnect all sources and sinks of this algorithm from anything they would be connected to.
   */
  void disconnectAll();

  /**
   * This is a non-blocking function.
   */
  AlgorithmStatus acquireData();
  void releaseData();

  virtual AlgorithmStatus process() = 0;

  /**
   * This function will be called when doing batch computations between each
   * file that is processed. That is, if your algorithm is some sort of state
   * machine, it allows you to reset it to its original state to process
   * another file without having to delete and reinstantiate it. This function
   * should not be called directly. Use resetNetwork instead to reset a network
   * of connected Algorithms.
   */
  virtual void reset();

 protected:
  /** Declare a Sink for this algorithm. The sink uses its default acquire/release size. */
  void declareInput(SinkBase& sink, const std::string& name, const std::string& desc);

  /** Declare a Sink for this algorithm. The sink uses @c n for the acquire/release size. */
  void declareInput(SinkBase& sink, int n, const std::string& name, const std::string& desc);

  /** Declare a Sink for this algorithm. The sink uses the given acquire/release size. */
  void declareInput(SinkBase& sink, int acquireSize, int releaseSize, const std::string& name, const std::string& desc);


  /** Declare a Source for this algorithm. The source uses its default acquire/release size. */
  void declareOutput(SourceBase& source, const std::string& name, const std::string& desc);

  /** Declare a Source for this algorithm. The source uses @c n for the acquire/release size. */
  void declareOutput(SourceBase& source, int n, const std::string& name, const std::string& desc);

  /** Declare a Source for this algorithm. The source uses the given acquire/release size. */
  void declareOutput(SourceBase& source, int acquireSize, int releaseSize, const std::string& name, const std::string& desc);



  bool _shouldStop;

  OutputMap _outputs;
  InputMap _inputs;

#ifdef DEBUGGING_ENABLED
  friend class essentia::scheduler::Network;

  /** number of times the process() method has been called */
  int nProcess;
#endif

};


} // namespace streaming
} // namespace essentia


// these #include have to go here, and not on top, because they need to have
// Algorithm defined first. Indeed, Source implementation (templated, so
// implementation goes into header) depends on PhantomBuffer (templated), which
// in turn depends on Algorithm for the synchronization for acquiring
// access to the buffer resources
#include "source.h"
#include "sink.h"

// also include template algorithms which are not in the factory, but are still
// useful most of the time
#include "devnull.h"


#endif // ESSENTIA_STREAMINGALGORITHM_H
