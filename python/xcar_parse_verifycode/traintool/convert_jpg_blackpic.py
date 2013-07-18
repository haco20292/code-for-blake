import os
from PIL import Image,ImageFilter,ImageEnhance
def ConvertAll():
	WIDTH = 320
	HEIGHT = 159
	MAXCOUNT = 10
	region = (0, 0, WIDTH, HEIGHT)
	pos = [0, 0, WIDTH, HEIGHT]
	count = 0
	newim = Image.new("L", ((WIDTH) * MAXCOUNT, HEIGHT))
	for f in os.listdir('jpgfiles'):
		#if f != '19.jpg':
			#continue
		if not f.endswith('.jpg'):
			continue
		oldfile = r'jpgfiles\%s' % f
		im = Image.open(oldfile)	#.convert('L').point(lambda x: 255 if x > 127 else 0)
		if im.size != (320, 240):
			print oldfile
			continue
		im = im.crop(region)
		im = im.filter(ImageFilter.MedianFilter())
		#enhancer = ImageEnhance.Contrast(im)
		#im = enhancer.enhance(2)
		im = ImageEnhance.Contrast(im).enhance(5)
		im = im.convert('L').point(lambda x: 255 if x > 127 else 0)
		newim.paste(im, pos)
		pos[0] += region[2] - region[0]
		pos[2] += region[2] - region[0]
		if(pos[0] >= newim.size[0]):
			#print pos, newim.size
			newfile = r'train\eng.xcar.exp%s.tif' % count
			count += 1
			newim.save(newfile)
			pos = list(region)
			print newfile
			
if __name__ == '__main__':
	ConvertAll()
		