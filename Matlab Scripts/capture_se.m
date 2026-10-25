%% 
plot_baseband = false;
plot_ft = false;
scans = 59;
fs = 50e3;
decim = 500;
freq = 4.52*10^6;

hold off;

for i = 1:scans
    mm = ftgrab(3000);
    samples = size(mm(1,1:end));
    samples = samples(2);
    time = linspace(0, samples/fs, samples);
    if(plot_baseband)
        plot(time(1:10:end), mm(1,1:10:end));
    elseif (plot_ft)
        dm = demodulation(freq, mm(1,1:end));
        ft = fft(dm(165:200));
        L = size(ft, 2);
        plot(fs/L*(-L/2:L/2-1),abs(fftshift(ft)),"LineWidth",3)
        title("fft Spectrum in the Positive and Negative Frequencies")
        xlabel("f (Hz)")
        ylabel("|fft(X)|")
        xlim([-30000,30000]);
    else
        % dm = demodulation(freq, mm(1,1:end));
        % plot(real(dm(4200:4400)), 'b');
        % hold on;
        % plot(imag(dm(4200:4400)), 'r');
        % hold off;
    end
    if (i ~= scans)
        pause(2)
    end
    save("te3/te3_" + string(i+35) + ".mat", "mm");
end
% clear all;