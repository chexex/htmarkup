#-*- coding: utf-8 -*-

import os
import re
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

    # WARNING: self.agent.markup(text) 
    def markup(self, text):
        if isinstance(text, str):
            text = text.decode('utf8')

        # escape all non cp1251 symbols (e.q. ß, Ö)
        text = text.encode('cp1251', 'xmlcharrefreplace')

        marked_text = self.agent.markup(text)

        # back to unicode
        marked_text = marked_text.decode('cp1251')
        
        # replace non cp1251 symbols
        result = re.sub('&#(\d+);', lambda m: unichr(int(m.group(1))), marked_text)
        return result

    def version(self):
        return self.agent.version()

colorizer = ColorizerAgent()
