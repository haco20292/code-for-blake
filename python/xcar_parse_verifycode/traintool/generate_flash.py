import win32com.client
import pythoncom
import os
if __name__ == '__main__':
	SWFToImage = win32com.client.Dispatch("SWFToImage.SWFToImageObject")
	#SWFToImage = CreateObject("SWFToImage.SWFToImageObject")
	for i, f in enumerate(os.listdir("files")):
		if not f.endswith(".swf"):
			continue
		filepath = os.path.abspath('.') + '\\files\\' + f
		if not os.path.exists(filepath):
			continue
		SWFToImage.InputSWFFileName = filepath
		SWFToImage.ImageOutputType = 1 #' set output image type to Jpeg (0 = BMP, 1 = JPG, 2 = GIF)
		SWFToImage.Execute_Begin() #' start conversion
		#SWFToImage.FrameIndex = 1 ' set frame index
		SWFToImage.Execute_GetImage()
		SWFToImage.SaveToFile(r"jpgfiles\%d.jpg" % i) #"movie_with_dynamic_data.jpg"
		SWFToImage.Execute_End() # ' end conversion
