import scipy.signal as sig
import numpy as np
from matplotlib.pyplot import *

def print_initializer(varname,x):
    s = 'Eigen::MatrixXcf %s(%d,1);\n%s << ' % (varname,len(x),varname)
    
    for i in range(len(x)):
        if i != 0:
            s += ',\n'

        s += 'Complex_t(%9.8f,%9.8f)' % (np.real(x[i]),np.imag(x[i]))

    s += ';'

    print s 


Fs = 20.0 #hz sample frequency
bw = 1.5 #Hz bandwidth of originator lowpass
Fc = 2.0#Hz center frequency of complex bandpass
numtaps = 101

Fb = 2*bw / Fs

B = sig.firwin(numtaps, Fb)
shifter = np.exp(1j*2 * Fc / Fs * np.pi*np.array(range(numtaps)))
Bs = B * shifter
Bsn = B * np.conj(shifter)


x = 2*np.random.rand(10000) + np.random.rand(10000)*1j
x = 2*x - 1.0

y = sig.lfilter(B,1,x)
ys = sig.lfilter(Bs,1,x)
ysn = sig.lfilter(Bsn,1,x)

f,h = sig.welch(y)
f,hs = sig.welch(ys)
f,hsn = sig.welch(ysn)

h = 10*np.log10(h)
hs = 10*np.log10(hs)
hsn = 10*np.log10(hsn)


print "green"
print_initializer('Bplus',Bs)

print "red"
print_initializer('Bminus',Bsn)


plot(f,h,f,hs,f,hsn); show()
