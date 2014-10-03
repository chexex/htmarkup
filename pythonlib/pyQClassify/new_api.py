#-*- coding: utf-8 -*-

from pyQClassify.libpyQClassify import Agent
import unittest


index_file = '/home/sites/health2.mail.ru/var/colorizer/common.idx'
config_file = '/home/sites/health2.mail.ru/var/colorizer/colorizer.xml'


class AgentTestCase(unittest.TestCase):

    def setUp(self):
        self.colorizer = Agent(config_file)

    def test_version(self):
        self.assertEqual(10, self.colorizer.version())

    def test_config(self):
        self.assertEqual(config_file, self.colorizer.config)

    def test_index_filename(self):
        self.assertEqual(index_file, self.colorizer.get_index_file_name())

    def test_markup(self):
        self.assertEqual(u'foo', self.colorizer.markup(u'foo'))

    def test_instances(self):
        Agent(config_file)
        Agent(config_file)
        Agent(config_file)
