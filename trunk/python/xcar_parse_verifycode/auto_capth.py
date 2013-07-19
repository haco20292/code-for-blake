import win32com.client
import pythoncom
import os
import urllib2
from PIL import Image,ImageFilter,ImageEnhance
def DownloadUrl(url, filename):
		#urllib.urlretrieve(url, filename)
		proxy = urllib2.ProxyHandler({'http': 'http://web-proxy.oa.com:8080'})
		opener = urllib2.build_opener(proxy)
		#opener.addheaders = [("User-Agent", r"Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; QQDownload 718; .NET CLR 2.0.50727; .NET4.0C; .NET4.0E)"),
		#	(r"Accept", r"image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/xaml+xml, application/x-ms-xbap, application/x-ms-application, */*"),
		#	("Cookie", r"__utma=247269531.1730131159.1349958885.1349958885.1349958885.1; Hm_lvt_53eb54d089f7b5dd4ae2927686b183e0=1349958884765; LDCLGFbrowser=37bf78b2-65b0-4605-a054-2bfd8fd44781"),
		#	("Accept-Encoding", "gzip, deflate")]
		content = opener.open(url).read()
		file(filename, 'w').write(content)
def Flash2Jpg(flashfile, jpgfile):
	SWFToImage = win32com.client.Dispatch("SWFToImage.SWFToImageObject")
	#SWFToImage = CreateObject("SWFToImage.SWFToImageObject")
	if not os.path.exists(flashfile):
		raise ValueError, "not exist file:%s" % flashfile
	SWFToImage.InputSWFFileName = flashfile
	SWFToImage.ImageOutputType = 1 #' set output image type to Jpeg (0 = BMP, 1 = JPG, 2 = GIF)
	SWFToImage.Execute_Begin() #' start conversion
	#SWFToImage.FrameIndex = 1 ' set frame index
	SWFToImage.Execute_GetImage()
	SWFToImage.SaveToFile(jpgfile) #"movie_with_dynamic_data.jpg"
	SWFToImage.Execute_End() # ' end conversion
def Jpg2TwoValue(jpgfile, tvaluefile):
	im = Image.open(jpgfile)	#.convert('L').point(lambda x: 255 if x > 127 else 0)
	im = im.filter(ImageFilter.MedianFilter())
	im = ImageEnhance.Contrast(im).enhance(5)
	im = im.convert('L').point(lambda x: 255 if x > 127 else 0)
	im.save(tvaluefile)
def Pic2String(picfile):
	ret = os.system('tesseract %s result -l xca' % picfile)
	return file('result.txt').readline()
def XcarUrl2Text(url):
	#os.environ['http_proxy'] = 'http://proxy.tencent.com:8080'
	swf = 'xcar_verifycode.swf'
	DownloadUrl(url, swf)
	jpg = 'xcar_verifycode.jpg'
	Flash2Jpg(swf, jpg)
	tvalue = 'xcar_verifycode.jpg'
	Jpg2TwoValue(jpg, tvalue)
	return Pic2String(tvalue)
def Flash2Text(flashfile):
	jpg = flashfile + '.jpg'
	Flash2Jpg(flashfile, jpg)
	Jpg2TwoValue(jpg, jpg)
	return Pic2String(jpg)
	
if __name__ == '__main__':
	#DownloadUrl('http://www.xcar.com.cn/bbs/include/image_pwd.php?ssid=1374051515', 'proxy.pac')
	print XcarUrl2Text('http://www.xcar.com.cn/bbs/include/image_pwd.php?ssid=1374051515')
	#print XcarUrl2Text('http://www.xcar.com.cn/')
