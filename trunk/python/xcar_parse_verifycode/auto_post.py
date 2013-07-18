#!/usr/bin/env python
# -*- coding: utf-8 -*-
import requests
from bs4 import BeautifulSoup 

class CPerson:
	INIT_URL = 'http://my.xcar.com.cn/logging.php?action=login&referer='
	LG_URL = 'http://my.xcar.com.cn/logging.php?action=login'
	def __init__(self):
		self.ss = requests.Session()
		self.ss.headers['User-Agent'] = r'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0;)'
	def _FillInput(self, dom, *pairs):
		'''dom => BeautifulSoup
		   pairs => (tagname, id), (tagname, id), ...
		'''
		result = {}
		for one in pairs:
			tagname, idname = one
			print tagname, idname
			v = dom.find(tagname, {'name':idname})
			result[idname] = v['value']
		return result
	def Login(self, username, pwd):
		c = BeautifulSoup(self.ss.get(CPerson.INIT_URL).text)
		params = self._FillInput(c, ('input', 'chash'), ('input', 'dhash'), ('input', 'ehash'), ('input', 'formhash'))
		import hashlib
		mm = hashlib.md5()
		mm.update(pwd)
		params['password'] = mm.hexdigest()
		params['username'] = username
		params['cookietime'] = '2592000'
		params['iphone'] = params['typelogin'] = ''
		params['referer'] = 'http://my.xcar.com.cn/'
		params['loginsubmit']='提交'
		result = self.ss.post(self.LG_URL, params)
		return result
	def GetUrl(self):
		result = self.ss.get('http://my.xcar.com.cn')

if __name__ == '__main__':
	one = CPerson()
	re = one.Login('tangb4c', '131457')
	one.GetUrl()
	print re.content
	print re.headers
		
		