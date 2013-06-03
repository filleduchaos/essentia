#!/usr/bin/env python

from essentia_test import *
from essentia._essentia import version
import os



def getYaml(filename):
    resultF = open(filename, 'r')
    result = resultF.read()
    resultF.close()
    os.remove('test.yaml')
    return result



class TestYamlOutput(TestCase):

    metadata = '\nmetadata:\n    version:\n        essentia: "' + version() + '"\n'


    def testSingleRealSingleKey(self):
        p = Pool()
        p.add('foo', 1.0)

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: [1]
'''

        self.assertEqual(result, expected)


    def testSingleStringSingleKey(self):
        p = Pool()
        p.add('foo', 'foo')

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: ["foo"]
'''

        self.assertEqual(result, expected)


    def testMultRealSingleKey(self):
        p = Pool()
        p.add('foo', 1.0)
        p.add('foo', 2.0)

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: [1, 2]
'''

        self.assertEqual(result, expected)


    def testMultStringSingleKey(self):
        p = Pool()
        p.add('foo', 'foo')
        p.add('foo', 'bar')

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: ["foo", "bar"]
'''

        self.assertEqual(result, expected)


    def testSingleRealSeparateKey(self):
        p = Pool()
        p.add('foo', 1.0)
        p.add('bar', 1.0)

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
bar: [1]

foo: [1]
'''

        self.assertEqual(result, expected)


    def testMultRealSeparateKey(self):
        p = Pool()
        p.add('foo', 1)
        p.add('foo', 2)
        p.add('bar', 1)
        p.add('bar', 2)

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
bar: [1, 2]

foo: [1, 2]
'''

        self.assertEqual(result, expected)


    def testSingleVectorStringSingleKey(self):
        p = Pool()
        p.add('foo', ['waf', 'needs', 'documentation'])

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: [["waf", "needs", "documentation"]]
'''

        self.assertEqual(result, expected)


    def testMultVectorStringSingleKey(self):
        p = Pool()
        p.add('foo', ['ubuntu', '8.10', 'released!'])
        p.add('foo', ['should', 'i try', 'kde?'])

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo: [["ubuntu", "8.10", "released!"], ["should", "i try", "kde?"]]
'''

        self.assertEqual(result, expected)


    def testMultVectorStringSeparateKey(self):
        p = Pool()
        p.add('foo', ['ubuntu', '8.10', 'released!'])
        p.add('foo', ['should', 'i try', 'kde?'])
        p.add('bar', ['make', 'sure', 'to'])
        p.add('bar', ['pimp', 'your', 'starcraft'])

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
bar: [["make", "sure", "to"], ["pimp", "your", "starcraft"]]

foo: [["ubuntu", "8.10", "released!"], ["should", "i try", "kde?"]]
'''

        self.assertEqual(result, expected)


    def testNestedKeys(self):
        p = Pool()
        p.add('foo.bar', 'foobar')
        p.add('foo.foo', 'barfoo')

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
foo:
    bar: ["foobar"]
    foo: ["barfoo"]
'''

        self.assertEqual(result, expected)


    def testComprehensive(self):
        p = Pool()
        mat = ones((2,2))
        p.add('reals.single', 2)
        p.add('reals.single', 4)
        p.add('reals.vec', [3,4,5])
        p.add('reals.vec', [5,6,7,8])
        p.add('reals.matrix', mat)
        p.add('strs.vec', ['foo', 'bar'])
        p.add('strs.vec', ['bar', 'foo'])
        p.add('really.long.key.name', 2008)
        p.add('really.long.something.else', 2009)
        p.add('strs.single', 'foo')
        p.add('strs.single', 'bar')

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
really:
    long:
        key:
            name: [2008]
        something:
            else: [2009]

reals:
    single: [2, 4]
    vec: [[3, 4, 5], [5, 6, 7, 8]]
    matrix: [[[1, 1], [1, 1]]]

strs:
    single: ["foo", "bar"]
    vec: [["foo", "bar"], ["bar", "foo"]]
'''

        self.assertEqual(result, expected)


    def testEmptyString(self):
        p = Pool()
        p.add('empty', '""')

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
empty: ["\\"\\""]
'''

        self.assertEqual(result, expected)


    def testRational(self):
        p = Pool()
        p.add('rational', 3.145)
        p.add('rational', -0.456)

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')

        # we have to check each line separately because rational numbers need
        # to be approximately compared
        lines = result.split('\n')

        self.assertEqual(len(lines), 7)

        self.assertEqual(lines[0], '')
        self.assertEqual(lines[1], 'metadata:')
        self.assertEqual(lines[2], '    version:')
        self.assertEqual(lines[3], '        essentia: "'+version()+'"')
        self.assertEqual(lines[4], '')
        # will check line 5 later
        self.assertEqual(lines[6], '')

        numbers = lines[5][11:-1]
        numbers = numbers.split(', ')

        self.assertEqual(len(numbers), 2)
        self.assertAlmostEqual(float(numbers[0]), 3.145)
        self.assertAlmostEqual(float(numbers[1]), -0.456)


    def testStereoSample(self):
        p = Pool()
        p.add('stereosample', (3, 6))
        p.add('stereosample', (-1, 2))

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + '''
stereosample: [{left: 3, right: 6}, {left: -1, right: 2}]
'''
        self.assertEqual(result, expected)


    def testEmptyPool(self):
        p = Pool()

        YamlOutput(filename='test.yaml')(p)

        result = getYaml('test.yaml')
        expected = self.metadata + ''''''
        self.assertEqual(result, expected)

    def testInvalidFile(self):
        p = Pool()
        self.assertRaises(RuntimeError, lambda: YamlOutput(filename='')(p))


suite = allTests(TestYamlOutput)

if __name__ == '__main__':
    TextTestRunner(verbosity=2).run(suite)
