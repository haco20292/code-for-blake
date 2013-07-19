#!/usr/bin/env python
# -*- coding: utf-8 -*-
import requests,re,random, logging, os
from bs4 import BeautifulSoup
import auto_capth

logging.basicConfig(filename = os.path.join(os.getcwd(), 'log.txt'), 
    level = logging.DEBUG, format = '%(asctime)s - %(levelname)s: %(message)s')
USERAGENT = r'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0;)'
class CPerson:
	INIT_URL = 'http://my.xcar.com.cn/logging.php?action=login&referer='
	LG_URL = 'http://my.xcar.com.cn/logging.php?action=login'
	BBS_URL ='http://www.xcar.com.cn/bbs/'
	BBS = ()
	def __init__(self):
		self.ss = requests.Session()
		self.ss.headers['User-Agent'] = USERAGENT
	def InitBBS(self):
		ul = self.GetDOM(CPerson.BBS_URL)
		#ul = requests.get(CPerson.BBS_URL)
		#ul.encoding = 'GB2312'
		#ul = BeautifulSoup(ul.text)
		ul = ul.find_all('a', href = re.compile(r'^http://www.xcar.com.cn/bbs/forumdisplay.php\?fid='))
		bbs = [int(x['href'].replace('http://www.xcar.com.cn/bbs/forumdisplay.php?fid=', '').replace('"', '')) for x in ul]
		CPerson.BBS = tuple(set(bbs) - set((267,58,715))) #remove some form(like banzhu, banwu)
	def RandomGetForumUrl(self):
		if len(CPerson.BBS) < 1:
			self.InitBBS()
		bbs = CPerson.BBS
		index = random.randrange(0,len(bbs))
		logging.debug('random for bbs:%d %d total:%d', index, bbs[index], len(bbs))
		return 'http://www.xcar.com.cn/bbs/forumdisplay.php?fid=%d' % bbs[index]
			
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
	def GetDOM(self, url, next_referer = True):
		'''
		get dom from url'''
		rsp = self.ss.get(url)
		if next_referer:		#for next request
			self.ss.headers['Referer'] = url

		rsp.encoding = 'GB2312'
		return BeautifulSoup(rsp.text)

	def GetThreads(self, url):
		'''
		return [{url:viewthread.php, author:xxx, count:xxx, pages:xxx},....]
		'''
		forum = self.GetDOM(url)
		logging.debug('GetThreads url:%s', url)
		#print url, forum.title
		threads = forum.find_all('table', class_='row')
		result = list()
		for one in threads:
			tu = one.find('a', href=re.compile(r'^viewthread.php\?tid=\d+'))
			info = one.find('td', class_='c4')
			author = one.find('a', rel='nofollow')
			pages = one.find_all('a', href = re.compile(r'page=\d+'))
			if tu and info and author:
				pages = int(pages.pop().text) if pages else 0
				result.append({'url':tu['href'], 'author':author.text, 'count':int(info.next_element), 'pages':pages})
		return result
	def GetReferComment(self, threadinfo):
		'''threadinfo => {url:viewthread.php, author:xxx, count:xxx, pages:xxx}
		return refercomment, page-dom
		'''
		ti = threadinfo
		pages = [0]
		if(ti['pages'] > 2):
			pages = range(ti['pages'] - 1)
			random.shuffle(pages)
		while pages:
			page = pages.pop()
			if page:
				pageurl = 'http://www.xcar.com.cn/bbs/%s&page=%d' % (ti['url'], page)
			else:
				pageurl = 'http://www.xcar.com.cn/bbs/%s' % ti['url']
			c = self.GetDOM(pageurl)
			logging.debug('thread url:%s', pageurl)
			all = c.find_all('table', class_ = 't_row')
			random.shuffle(all)
			for one in all:
				author = one.find('a', rel='nofollow').text
				comment = one.find('div', id = re.compile('^message'))
				if not comment or author == ti['author']:	#skip author
					logging.debug('skip comment. %s, author:%s ti.author:%s', comment, author, ti['author'])
					continue
				if comment.find('div', class_ = 'msgbody'): #skip refer post
					continue
				rr = comment.text.strip()
				if len(rr) > 5 and len(rr) < 50:
					logging.debug('with comment:%s', rr)
					return c, rr
		return None,None

	def GetVerifycode(self, ssid):
		rsp = self.ss.get('http://www.xcar.com.cn/bbs/include/image_pwd.php?ssid=' + ssid)
		filename = '%s.swf' % ssid
		file(filename, 'w').write(rsp.content)
		return auto_capth.Flash2Text(filename)
		
	def AddPost(self, dom, comment):
		params = self._FillInput(dom, ('input', 'ssid'), ('input', 'usesig'), ('input', 'formhash'))
		params['previewpost'] = ''
		params['subject']=''
		params['seccodeverify'] = self.GetVerifycode(params['ssid'])
		params['message'] = comment.encode('GB2312')
		url = dom.find('form', id = 'postform')
		url = '%s%s' % (CPerson.BBS_URL, url['action'])
		result = self.ss.post(url, params)
		return result
	def Login(self, username, pwd):
		c = self.GetDOM(CPerson.INIT_URL)
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
		logging.debug('Login as:%s :%s', username, result.url)
		if result.url != self.LG_URL:
			return
		raise ValueError, "login failed. rsp url:%s" % result.url

if __name__ == '__main__':
	one = CPerson()
	one.InitBBS()
	one.Login('blaketang', '131457')
	forum = one.RandomGetForumUrl()
	threads = one.GetThreads(forum)
	for x in threads:
		if x['count'] > 20:
			dom, ce = one.GetReferComment(x)
			if not ce:
				continue
			result = one.AddPost(dom, ce)
			break


		
		
