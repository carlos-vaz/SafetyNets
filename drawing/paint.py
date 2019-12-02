import cv2
import numpy as np
import socket

# mouse callback function
def draw_line(event,x,y,flags,param):
	global x_old, y_old
	global down
	if event == cv2.EVENT_LBUTTONDOWN:
		cv2.line(img,(x,y), (x_old,y_old),(245,0,0),12,8)
		x_old = x
		y_old = y
		down = True
	elif event == cv2.EVENT_MOUSEMOVE:
		if down:
			cv2.line(img,(x,y), (x_old,y_old),(245,0,0),12,8)
		x_old = x
		y_old = y
	elif event == cv2.EVENT_LBUTTONUP:
		down = False
	

def image_resize(image, width = None, height = None, inter = cv2.INTER_AREA):
    # initialize the dimensions of the image to be resized and
    # grab the image size
    dim = None
    (h, w) = image.shape[:2]

    # if both the width and height are None, then return the
    # original image
    if width is None and height is None:
        return image

    # check to see if the width is None
    if width is None:
        # calculate the ratio of the height and construct the
        # dimensions
        r = height / float(h)
        dim = (int(w * r), height)

    # otherwise, the height is None
    else:
        # calculate the ratio of the width and construct the
        # dimensions
        r = width / float(w)
        dim = (width, int(h * r))

    # resize the image
    resized = cv2.resize(image, dim, interpolation = inter)

    # return the resized image
    return resized	


# Create a black image, a window and bind the function to window
while(1):
	black_img_dimensions = 200
	x_old = -1
	y_old = -1
	down = False
	img = np.zeros((black_img_dimensions,black_img_dimensions,3), np.uint8)
	cv2.namedWindow('image')
	cv2.setMouseCallback('image',draw_line)
	
	while(1):
		cv2.imshow('image',img)
		if cv2.waitKey(20) & 0xFF == ord('\r'):
			break

	# Calculate bounding rectangle
	first = True
	for row in range(0,black_img_dimensions):
		for pix in range(0,black_img_dimensions):
			if img[row][pix][0] > 0:
				if first:
					points = np.array([[pix,row]])
					first = False
				points = np.append(points, [[pix,row]], axis=0)
	print(points)
	rect = cv2.boundingRect(points)
	x,y,w,h = rect
	cv2.rectangle(img,(x,y),(x+w,y+h),(0,255,0),2)

	# Center bouding rectangle + contents
	center = black_img_dimensions/2
	img = np.zeros((black_img_dimensions,black_img_dimensions,3), np.uint8)
	move_x = center - (x+w/2);
	move_y = center - (y+h/2);
	for pts in range(0,len(points)):
		newX = points[pts][0] + move_x
		newY = points[pts][1] + move_y
		img[newY][newX][0] = 245

	# Re-scale number relative to image borders
	justNumber = np.zeros((h,w,3),np.uint8)
	for row in range(0,h):
		for pix in range(0,w):
			justNumber[row][pix] = img[(int)((black_img_dimensions-h)/2)+row][(int)((black_img_dimensions-w)/2)+pix]
	AdjustedRatio = 0.55
	justNumber = image_resize(justNumber, height = (int)(black_img_dimensions*AdjustedRatio))
	img_final = np.zeros((black_img_dimensions,black_img_dimensions,3), np.uint8)
	for row in range(0,(int)(black_img_dimensions*AdjustedRatio)):
		for pix in range(0,(int)(black_img_dimensions*AdjustedRatio*w/h)):
			img_final[(int)(black_img_dimensions*(1-AdjustedRatio)/2)+row][(int)(black_img_dimensions*(1-AdjustedRatio*w/h)/2)+pix] = justNumber[row][pix]

	# Prepare to send image (resize)
	img = image_resize(img_final, height=28)
	img = img[:,:,0]
	
	# Send message to signal server to check file
	message = 'e'
	f = open('image', 'w')
	for row in range(0,28):
		for pix in range(0,28):
			num = int(img[row][pix]/25.5)
			f.write(str(num)+"\n")
			message += (str(num)) + " "
	f.close()
	
	HOST = '127.0.0.1'
	PORT = 8081
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST,PORT))
	
	s.sendall(message)
	s.close()


	while(1):
		cv2.imshow('image',img_final)
		if cv2.waitKey(20) & 0xFF == 27:
			break

	cv2.destroyAllWindows()



