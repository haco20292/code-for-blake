import os, glob
def CreateBox(name, ext):
	tiffile = name + ext
	os.system('tesseract %s %s -l xca batch.nochop makebox' % (tiffile, name))
	os.startfile(tiffile)
	boxfile = '%s.box' % name
	lines = file(boxfile).readlines()
	letters = [x[0] for x in lines]
	result = ''
	for i in range(0, len(letters), 4):
		result += ''.join(letters[i:i+4])
		result += ' '
	fixed = raw_input(result + '\n')
	if fixed != 'c':
		fixed = [x.upper() for x in fixed if x != ' ']
		if len(fixed) != len(lines):
			raise ValueError, "not equal len. %d => %d" % (len(fixed) != len(lines))
		for i in range(0, len(lines)):
			lines[i] = fixed[i] + lines[i][1:]
		file(boxfile, 'w').writelines(lines)
	else:
		print 'All right.continue...'
def CreateTrainData(lang, name):
	os.system(r'tesseract %s.tif %s nobatch box.train' % (name, name))
	allbox = ' '.join(glob.glob('*.box'))
	os.system(r'unicharset_extractor.exe %s' % allbox)
	alltr = ' '.join(glob.glob('*.tr'))
	os.system(r'mftraining -F font_properties -U unicharset -O %s.unicharset %s' % (lang, alltr))
	os.system(r'cntraining.exe %s' % alltr)
	os.system(r'move /Y normproto %s.normproto' % lang)
	os.system(r'move /Y inttemp %s.inttemp' % lang)
	os.system(r'move /Y pffmtable %s.pffmtable' % lang)
	os.system(r'move /Y shapetable %s.shapetable' % lang)
	os.system(r'combine_tessdata %s.' % lang)
	raw_input("Pause.any key to continue...")
	os.system(r'copy %s.traineddata "C:\Program Files\Tesseract-OCR\tessdata\xca.traineddata"' % lang)
def DealAllTif():
	'''
	for f in os.listdir('.'):
		if not f.endswith('.tif') or not f.startswith('eng.xcar.exp'):
			continue
	'''
	for x in range(7, 500):
		f = 'eng.xcar.exp%d.tif' % x
		if not os.path.exists(f):
			print 'not exist', f
			continue
		name, ext = os.path.splitext(f)
		lang = name.split('.')[0]
		CreateBox(name, ext)
		CreateTrainData(lang, name)
		
if __name__ == '__main__':
	DealAllTif()