function filtered_bb = demodulation(freq, mm)
    L=length(mm);
    Fs = 50*10^6;
    t = 0:1/Fs:(L-1)/Fs;
    dm = exp(1i*2*pi*t*freq);
    mm = double(mm(1,:))-mean(mm(1,:));
    bb = double(mm).*dm;
    

    filtered_bb = decimate(bb,100);
    filtered_bb = decimate(filtered_bb, 5);
end