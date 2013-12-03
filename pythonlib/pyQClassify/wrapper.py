#-*- coding: utf-8 -*-

import os
from pyQClassify.libpyQClassify import Agent, QClassifyError

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

        return self.agent.classifyPhrase(phrase)
    
    def loadConfig(self, path):
        if not os.path.exists(path):
            raise OSError(2, "No such file or directory", path)

        res = self.agent.loadConfig(path)
        self.configured = True
        return res

    def markup(self, text):
        if isinstance(text, unicode):
            text = text.encode('utf8')
        return self.agent.markup(text)

    def version(self):
        return self.agent.version()

colorizer = ColorizerAgent()
