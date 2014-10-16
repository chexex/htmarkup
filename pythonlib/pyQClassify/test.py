#-*- coding: utf-8 -*-

from pyQClassify import QClassifyAgent, QClassifyError
import unittest


index_file = 'data/index'
config_file = 'data/config.xml'


class QClassifyAgentTestCase(unittest.TestCase):

    def setUp(self):
        self.colorizer = QClassifyAgent(config_file)
        self.colorizer.index2file()

    def test_no_config(self):
        self.assertRaises(QClassifyError, QClassifyAgent, '/tmp/non_exists.xml')

    def test_wrong_config(self):
        self.assertRaises(QClassifyError, QClassifyAgent, '/tmp/wrong_config.xml')

    def test_version(self):
        self.assertEqual(10, self.colorizer.version())

    def test_config(self):
        self.assertEqual(config_file, self.colorizer.config)

    def test_get_index(self):
        self.assertEqual(index_file, self.colorizer.get_index())

    def test_markup(self):
        self.assertEqual(u'<a href="/drug/rubric/A06/">Слабительное</a>', self.colorizer.markup(u'Слабительное'))

    def test_error_markup(self):
        self.assertRaises(QClassifyError, self.colorizer.markup, u'使用安全套,虽然你学中文')

    def test_index2file(self):
        self.assertEqual(None, self.colorizer.index2file())

    def test_no_index_config(self):
        colorizer = QClassifyAgent('data/no_index_config.xml')
        self.assertRaises(QClassifyError, colorizer.markup, u'Слабительное')

    def test_many_instances(self):
        QClassifyAgent(config_file)
        QClassifyAgent(config_file)
        QClassifyAgent(config_file)
