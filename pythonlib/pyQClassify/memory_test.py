#-*- coding: utf-8 -*-

from pyQClassify import QClassifyAgent
import gc
import os


def get_mem():
    a = os.popen('ps -p %d -o %s | tail -1' % (os.getpid(), "vsize,rss,pcpu")).read()
    a = a.split()
    return (int(a[0]), int(a[1]))


def main():
    config_file = 'data/config.xml'
    text = open('data/text').read().decode('utf8')

    for i in xrange(10 ** 5):
        colorizer = QClassifyAgent(config_file)
        colorizer.markup(text)

        gc.collect()
        print get_mem()


if __name__ == "__main__":
    main()
