#!/usr/bin/env python
# -*- coding: utf-8 -*-
import requests,re,random, logging, os, time, sys, traceback
from bs4 import BeautifulSoup
import auto_capth

USERAGENT = r'Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0;)'
class CPerson:
	INIT_URL = 'http://my.xcar.com.cn/logging.php?action=login&referer='
	LG_URL = 'http://my.xcar.com.cn/logging.php?action=login'
	BBS_URL ='http://www.xcar.com.cn/bbs/'
	PROFILE_URL= 'http://my.xcar.com.cn/'
	BBS = ()
	def __init__(self):
		self.ss = requests.Session()
		self.ss.headers['User-Agent'] = USERAGENT
		self.username = ''
		self.credit = 0			#credit count
		self.tids = set()	#used tid
		self._addcount = 0		#the count of calling AddPost
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
	def GetMyCredits(self, html = None):
		if not html:
			html = self.GetDOM(CPerson.PROFILE_URL)
		cc = html.find('em', id= 'mycredits', text = re.compile(ur'用户被禁止发言'))
		if cc:
			print >>sys.stderr, unicode(cc.text)
			sys.exit(1)
		credit = html.find('em', id = 'mycredits')
		if credit:
			self.credit = int(credit.text)
			logging.debug('my credit is:%d', self.credit)
		return self.credit

	def GetThreads(self, url):
		'''
		return [{url:viewthread.php, author:xxx, count:xxx, pages:xxx},....]
		'''
		forum = self.GetDOM(url)
		#logging.debug('GetThreads url:%s', url)
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
				last_author = one.find('td', class_= 'c5').find('a').text
				result.append({'url':tu['href'], 'author':author.text, 'count':int(info.next_element), 'pages':pages, 'last_author':last_author})
		return result

	def GetReferComment(self, threadinfo):
		'''threadinfo => {url:viewthread.php, author:xxx, count:xxx, pages:xxx}
		return refercomment, page-dom
		'''
		ti = threadinfo
		tid = re.search('tid=(\d+)', ti['url']).group(1)
		tid = int(tid)
		print tid
		if tid in self.tids:
			logging.debug('tid %d has existed.', tid)
			return None, None
		else:
			self.tids.add(tid)
		pages = [0]
		if(ti['pages'] > 2):
			pages = range(ti['pages'] - 1)
			random.shuffle(pages)
		pages.append(ti['pages'])	#for check exists self in the last page 
		while pages:
			page = pages.pop()
			if page:
				pageurl = 'http://www.xcar.com.cn/bbs/%s&page=%d' % (ti['url'], page)
			else:
				pageurl = 'http://www.xcar.com.cn/bbs/%s' % ti['url']
			c = self.GetDOM(pageurl)
			#logging.debug('thread url:%s', pageurl)
			have_self = c.find('a', href = re.compile('username=%s' % self.username))
			if have_self:
				logging.debug('exist username %s. skip it', self.username)
				return None, None
			if page == ti['pages']: #skip last pages
				continue
			all = c.find_all('table', class_ = 't_row')
			random.shuffle(all)
			for one in all:
				author = one.find('a', rel='nofollow').text
				comment = one.find('div', id = re.compile('^message'))
				if not comment or author == ti['author']:	#skip author
					logging.debug('skip comment.')
					continue
				if comment.find('div', class_ = 'msgbody'): #skip refer post
					continue
				rr = comment.text.strip()
				if len(rr) > 5 and len(rr) < 50:
					logging.debug('with comment:%s', rr)
					return c, rr
		return None,None

	def GetVerifycode(self, ssid):
		if self.credit > 1000:
			return None
		if self._addcount % 100 == 0:
			print 'Here'
			self.GetMyCredits()
		rsp = self.ss.get('http://www.xcar.com.cn/bbs/include/image_pwd.php?ssid=' + ssid)
		#filename = '%s.swf' % ssid
		filename = self.username + '.swf'
		file(filename, 'w').write(rsp.content)
		code = auto_capth.Flash2Text(filename)
		logging.debug('verify code:%s len:%d' % (code, len(code)))
		return code
	def AddPost(self, dom, comment):
		params = self._FillInput(dom, ('input', 'ssid'), ('input', 'usesig'), ('input', 'formhash'))
		params['previewpost'] = ''
		params['subject']=''
		params['seccodeverify'] = self.GetVerifycode(params['ssid'])
		params['message'] = comment.encode('GB2312')
		url = dom.find('form', id = 'postform')
		url = '%s%s' % (CPerson.BBS_URL, url['action'])
		result = self.ss.post(url, params)
		if not result.headers.get('Refresh'):
			logging.debug('add post failed. headers:%s', result.headers)
		self._addcount += 1
		return result
	def Login(self, username, pwd):
		self.username = username
		c = self.GetDOM(CPerson.INIT_URL)
		params = self._FillInput(c, ('input', 'chash'), ('input', 'dhash'), ('input', 'ehash'), ('input', 'formhash'))
		import hashlib
		mm = hashlib.md5()
		mm.update(pwd)
		params['password'] = mm.hexdigest()
		params['username'] = username
		params['cookietime'] = '2592000'
		params['iphone'] = params['typelogin'] = ''
		params['referer'] = CPerson.PROFILE_URL
		params['loginsubmit']='提交'
		result = self.ss.post(self.LG_URL, params)
		if result.url == CPerson.PROFILE_URL: #login succ
			logging.debug('Login as:%s :%s', username, result.url)
			html = BeautifulSoup(result.content, from_encoding='GB2312')
			self.GetMyCredits(html)
			return result
		raise ValueError, "login failed. rsp url:%s" % result.url
if __name__ == '__main__':
	one = CPerson()
	if len(sys.argv) < 3:
		print '%s username passwd' % sys.argv[0]
		exit()

	one.username = sys.argv[1]
	one.passwd = sys.argv[2]
	logging.basicConfig(filename = os.path.join(os.getcwd(), one.username + '.log'), 
    	level = logging.DEBUG, format = '%(asctime)s - %(levelname)s - %(funcName)s: %(message)s')

	one.InitBBS()
	one.Login(one.username, one.passwd)
	one.GetMyCredits()
	count = 0
	while True:
		forum = one.RandomGetForumUrl()
		threads = one.GetThreads(forum)
		for x in threads:
			try:
				if x['last_author'] != one.username and x['count'] > 25:
					dom, ce = one.GetReferComment(x)
					if not ce or not dom:
						continue
					
					result = one.AddPost(dom, ce)
					count += 1
					msg = u'http://www.xcar.com.cn/bbs/%s\n%dcomments:%s' % (x['url'], count, unicode(ce))
					logging.info(msg)
					print msg
					time.sleep(random.randrange(21, 30))
			except Exception, e:				
				#exstr = traceback.format_exc()
				exc_type, exc_value, exc_traceback = sys.exc_info()
				lines = traceback.format_exception(exc_type, exc_value, exc_traceback)
				print ''.join('!! ' + line for line in lines)  # Log it or whatever here
				logging.error('error happend. %s', e)

		
