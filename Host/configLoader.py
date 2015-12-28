#!/usr/bin/python
# 
# Open the default rtu.ini file and load the
# contents into the local redis database
#
import ConfigParser
import redis


def main():
    config = ConfigParser.ConfigParser()
    config.read(['rtu.ini'])

    r=redis.Redis()
    dir(r)
    item={}
    name={}

    for s in config.sections():
        print s
        pin = config.getint(s,'pin')
        item['pin']= pin

        item['dir']= config.get(s,'dir')
        item['type']= config.get(s,'type')

        r.hmset(s,item)
        r.set(pin,s)



main()

