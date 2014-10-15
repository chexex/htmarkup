#-*- coding: utf-8 -*-

from pyQClassify.libpyQClassify import Agent, QClassifyError
import unittest


index_file = '/home/sites/health2.mail.ru/var/colorizer/common.idx'
config_file = '/home/sites/health2.mail.ru/var/colorizer/colorizer.xml'


class AgentTestCase(unittest.TestCase):

    def setUp(self):
        self.colorizer = Agent(config_file)

    def test_wrong_config(self):
        self.assertRaises(QClassifyError, Agent, '/tmp/non_exists.xml')

    def test_version(self):
        self.assertEqual(10, self.colorizer.version())

    def test_config(self):
        self.assertEqual(config_file, self.colorizer.config)

    def test_get_index(self):
        self.assertEqual(index_file, self.colorizer.get_index())

    def test_markup(self):
        self.assertEqual(u'<a href="/drug/rubric/A06/">Слабительное</a>', self.colorizer.markup(u'Слабительное'))

    def test_index2file(self):
        self.assertEqual(None, self.colorizer.index2file())

    def test_instances(self):
        Agent(config_file)
        Agent(config_file)
        Agent(config_file)
