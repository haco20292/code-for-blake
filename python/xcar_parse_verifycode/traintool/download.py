import urllib
def DownloadUrl(url, filename):
	for i in xrange(500):
		urllib.urlretrieve(url, "%s.swf" % i)
if __name__ == '__main__':
	url=r'http://www.xcar.com.cn/bbs/include/image_pwd.php?ssid=1373629126'
	filename='blaketang.swf'
	DownloadUrl(url, filename)
