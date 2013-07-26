#!/usr/bin/env python
# -*- coding: utf-8 -*-

import requests, chardet, re,sys, os
from bs4 import BeautifulSoup
from urlparse import urlsplit
ss = requests.Session()
baseurl = 'http://www.pingshu8.com/'
downurl = r'http://www.pingshu8.com/down_%s.html'
initurl = baseurl + r'bzmtv_Inc/Zjhits.asp?SpecialID=%s'
savedir = '.'
lastext = '.mp3'
def GetDOM(url):
	rsp = ss.get(url)
	if rsp.status_code != 200:
		raise ValueError, "code:%d url:%s len:%d" % (rsp.status_code, url, len(rsp.content))
	#rsp.encoding = chardet.detect(rsp.content)['encoding']
	#print rsp.encoding
	rsp.encoding = 'GBK'
	return BeautifulSoup(rsp.text)
def GetDownInfo(pageurl):
	dom = GetDOM(pageurl)
	div = dom.find('div', class_ = 'list4')
	urls = div.find_all('a', href = re.compile(r'^/play_\d+.html'))
	result = {}
	for one in urls:
		k = re.search(r'\d+', one['href']).group()
		result[k] = one.text
	return result
def CombineFileName(name, downurl = None):
	global lastext
	if downurl:
		ext = urlsplit(downurl).path
		ext = os.path.splitext(ext)[1]
		ext = ext.lower()
		if ext and ext != lastext:
			print 'found new ext:%s, old ext:%s' % (ext, lastext)
			lastext = ext

	filename = os.path.join(savedir, name + lastext)
	if os.path.exists(filename) and os.path.getsize(filename) > 1024 * 1024 * 3:
		print 'exists', filename, 'size:', os.path.getsize(filename)/(1024.0*1024), 'M'
		return None
	return filename

def DownloadOne(mid, name):
	filename = CombineFileName(name)
	if not filename: return
	burl = downurl % mid
	rsp = GetDOM(burl)
	#du = rsp.find('a', href=re.compile(r'download.pingshu8.com'))
	du = rsp.select('td > div > a[href]')[0]
	if du:

		durl = du['href']
		filename = CombineFileName(name, durl)
		if not filename: return
		#tmp = os.path.join(savedir, name + '.mp3')
		#if os.path.exists(tmp):
		#	os.rename(tmp, filename)
		#	print 'move', tmp, 'to', filename
		#return
		print 'downloading ', name, '...\t',
		sys.stdout.flush()
		#print durl
		#sys.exit(1)
		cc = ss.get(durl)
		if len(cc.content) < 1024 * 1024 * 2 or cc.status_code != 200:
			raise ValueError, "filename:%s url:%s rspcode:%d download file size:%d small. " % (filename, durl, cc.status_code, len(cc.content))
		file(filename, 'w').write(cc.content)
		print 'succ.', name
	else:
		raise ValueError, "url:%s failed. \n%s" % (burl, rsp.text)
def GetPages(dom):
	pages = ()
	ss = dom.find('select', {'name':'turnPage'}).find_all('option')
	r2 = [x['value'] for x in ss]
	global savedir
	savedir = dom.title.text.replace(u'评书网', u'').replace(u'评书 ', u'').replace(u' ', u'')
	if not savedir:
		raise ValueError, "empty savedir. len:%d" % len(savedir)
	if not os.path.exists(savedir):
		os.mkdir(savedir)
	return r2
def InitCookieByUrl(url):
	'''url should like :
		http://www.pingshu8.com/MusicList/mmc_15_114_1.Htm'''
	coms = url.split('_')
	if len(coms) != 4:
		raise ValueError, "url:%s not valid." % url
	pingshu_id = coms[2]
	_iurl = initurl % pingshu_id
	rsp = GetDOM(_iurl) #init cookie finished


def Download(url):
	'''url should like :
		http://www.pingshu8.com/MusicList/mmc_15_114_1.Htm'''
	InitCookieByUrl(url)
	dom = GetDOM(url)
	pages = GetPages(dom)
	for p in pages:
		durl = baseurl + p
		di = GetDownInfo(durl)
		for one in di.keys():
			DownloadOne(one, di[one])

		


if __name__ == '__main__':
	if len(sys.argv) < 3:
		print 'download.py base-dir url'
		print 'url like:http://www.pingshu8.com/MusicList/mmc_15_114_1.Htm'
		sys.exit(1)
	basedir = sys.argv[1]	
	os.chdir(basedir)
	print 'current work dir:', os.getcwd()
	#savedir = sys.argv[1]
	try:
		for url in sys.argv[2:]:
			Download(url)
	except Exception, e:
		print unicode(e)

