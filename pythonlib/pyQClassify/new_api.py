#-*- coding: utf-8 -*-

from pyQClassify.libpyQClassify import Agent
import unittest


config_file = '/home/sites/health2.mail.ru/var/colorizer/colorizer.xml'
config_file = 111


class V8TestCase(unittest.TestCase):

    def setUp(self):
        self.colorizer = Agent(config_file)

    def test_version(self):
        self.assertEqual(10, self.colorizer.version())

    def test_config(self):
        self.assertEqual(config_file, self.colorizer.config)
