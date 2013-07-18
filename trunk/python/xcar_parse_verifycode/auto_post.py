#!/usr/bin/env python
# -*- coding: utf-8 -*-
import requests,re,random
from bs4 import BeautifulSoup 

USERAGENT = r'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0;)'
class CPerson:
	INIT_URL = 'http://my.xcar.com.cn/logging.php?action=login&referer='
	LG_URL = 'http://my.xcar.com.cn/logging.php?action=login'
	BBS_URL ='http://www.xcar.com.cn/bbs'
	BBS = ()
	def __init__(self):
		self.ss = requests.Session()
		self.ss.headers['User-Agent'] = USERAGENT
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
	@staticmethod
	def InitBBS():
		ul = BeautifulSoup(requests.get(CPerson.BBS_URL, headers = {'User-Agent':USERAGENT}).content)
		ul = ul.find_all('a', href = re.compile(r'^http://www.xcar.com.cn/bbs/forumdisplay.php\?fid='))
		bbs = [int(x['href'].replace('http://www.xcar.com.cn/bbs/forumdisplay.php?fid=', '').replace('"', '')) for x in ul]
		CPerson.BBS = tuple(set(bbs) - set((267,58,715))) #remove some form(like banzhu, banwu)
	@staticmethod
	def RandomGetForumUrl():
		if len(CPerson.BBS) < 1:
			CPerson.InitBBS()
		bbs = CPerson.BBS
		return 'http://www.xcar.com.cn/bbs/forumdisplay.php?fid=%d' % bbs[random.randint(0,len(bbs))]
			
	def GetThreads(self, url):
		'''
		return [{url:viewthread.php, author:xxx, count:xxx},....]
		'''
		forum = BeautifulSoup(self.ss.get(url).text)
		print url, forum.title
		threads = forum.find_all('table', class_='row')
		result = list()
		for one in threads:
			tu = one.find('a', href=re.compile(r'^viewthread.php\?tid=\d+'))
			info = one.find('td', class_='c4')
			author = one.find('a', rel='nofollow')
			if tu and info and author:
				result.append({'url':tu['href'], 'author':author.text, 'count':int(info.next_element)})
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
		if result.url != self.LG_URL:
			return
		raise ValueError, "login failed. rsp url:%s" % result.url
	def GetUrl(self):
		result = self.ss.get('http://my.xcar.com.cn')

if __name__ == '__main__':
	one = CPerson()
	#one.Login('tangb4c', '131457')
	forum = one.RandomGetForumUrl()
	threads = one.GetThreads(forum)
	print threads

		
		
