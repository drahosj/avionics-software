def calculate(UT, UP):
	# Calibration Constants
	AC1 = 8435
	AC2 = -1232
	AC3 = -14355
	AC4 = 33977
	AC5 = 25552
	AC6 = 17158
	
	B1 = 6515
	B2 = 51
	
	MB = -32768
	MC = -11786
	MD = 2484
	
	OSS = 0
	
	# Temperature Calculation
	X1 = (UT - AC6) * AC5 / 2**15
	X2 = MC * 2**11 / (X1 + MD) 
	B5 = X1 + X2
	T = (B5 + 8) / 2**4
	
	# Pressure Calculation
	B6 = B5 - 4000
	X1 = (B2 * (B6 * B6 / 2**12)) / 2**11
	X2 = AC2 * B6 / 2**11
	X3 = X1 + X2
	B3 = (((AC1 * 4 + X3) << OSS) + 2 ) / 4
	X1 = AC3 * B6 / 2**13
	X2 = (B1 * (B6 * B6 / 2**12)) / 2**16
	X3 = ((X1 + X2) + 2) / 2**2
	B4 = numpy.uint32 (AC4 * (X3 + 32768) / 2**15)
	B7 = (numpy.uint32 (UP) - B3) * (50000 >> OSS)
	P = numpy.uint32 ((B7 * 2) / B4)
	X1 = (P / 2**8) * ( P / 2**8)
	X1 = int (X1 * 3038) / 2**16
	X2 = int (-7357 * P) / 2**16
	pressure = P + (X1 + X2 +3791) / 2**4
	
	return (T/10.0)*9.0/5.0 + 32, P
