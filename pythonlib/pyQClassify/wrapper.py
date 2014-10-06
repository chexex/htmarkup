#-*- coding: utf-8 -*-

import os
from pyQClassify.libpyQClassify import Agent, QClassifyError
import logging


logger = logging.getLogger(__name__)


class ColorizerAgent(object):
    def __init__(self):
        self.agent = Agent()

        # configuration file loaded
        self.configured = False

        # ready to mark up
        self.ready = False

    def getIndexFileName(self):
        return self.agent.getIndexFileName()

    def index2file(self):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        return self.agent.index2file()

    def initMarkup(self):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        index_file = self.getIndexFileName()
        if not os.path.exists(index_file):
            raise OSError(2, "No such file or directory", index_file)

        res = self.agent.initMarkup()
        self.ready = True
        return res

    reinitMarkup = initMarkup

    def classifyPhrase(self, phrase):
        if not self.configured:
            raise QClassifyError('Colorizer is not configured')

        if isinstance(phrase, unicode):
            phrase = phrase.encode('utf8')

        return self.agent.classifyPhrase(phrase)

    def loadConfig(self, path):
        if not os.path.exists(path):
            raise OSError(2, "No such file or directory", path)

        res = self.agent.loadConfig(path)
        self.configured = True
        return res

    def markup(self, text):
        try:
            return self.agent.markup(text)
        except QClassifyError, e:
            logger.error(e)
            return text

    def firstForm(self, word):
        is_unicode = False

        if isinstance(word, unicode):
            is_unicode = True
            word = word.encode('utf8')

        first_form = self.agent.firstForm(word)

        if is_unicode:
            first_form = first_form.decode('utf8')

        return first_form

    def version(self):
        return self.agent.version()

colorizer = ColorizerAgent()
