%% 
plot_baseband = false;
plot_ft = false;
scans = 21;
fs = 50e6;
decim = 100;
freq = 4.54*10^6;

hold off;

for i = 1:scans
    mm = ftgrab(300);
    samples = size(mm(1,1:end));
    samples = samples(2);
    time = linspace(0, samples/fs, samples);
    figure(1);
    if(plot_baseband)
        plot(time(1:decim:end), mm(1,1:decim:end));
    elseif (plot_ft)
        dm = demodulation(freq, mm(1,1:end));
        ft = fft(dm(165000:200000));
        L = size(ft)
        L = L(2)
        plot(fs/L*(-L/2:L/2-1),abs(fftshift(ft)),"LineWidth",3)
        title("fft Spectrum in the Positive and Negative Frequencies")
        xlabel("f (Hz)")
        ylabel("|fft(X)|")
        xlim([-40000,20000]);
    else
        dm = demodulation(freq, mm(1,1:end));
        plot(time(1:decim:end), real(dm(1:decim:end)), 'b');
        hold on;
        plot(time(1:decim:end), imag(dm(1:decim:end)), 'r');
        hold off;
        ylim([-1500,1500]);
        % xlim([30000,35000]);
    end
    if (i ~= scans)
        pause(4)
    end
    save("scan" + string(i) + ".mat", "mm");
end
% clear all;