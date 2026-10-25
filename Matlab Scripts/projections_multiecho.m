mm = load('proj_8p1.mat').mm(1,:);
freq = 4.507e6;
fs = 50e6/1000;
necho = 8;
efts = zeros(necho,36);
plotft = true;
plotdm = false;


for i = 1:necho
    echo = mm(90000 + (i-1)*180001:90000+ i*180001);
    % plot(mm(90000 + (i-1)*180001:90000+ i*180001))
    dm = demodulation(freq, echo);
    if(plotdm)
        plot(real(dm))
    end
    efts(i,:) = fftshift(fft(dm(75:110)));
    efts(i,:) = efts(i,:)/mean(abs(efts(i,:)));
    L = size(efts(i,:), 2);
    freqs = fs/L*(-L/2:L/2-1);
    efts(i,:) = efts(i,:).*exp(1i*2*pi*0.0018*freqs);
    if(plotft && (i==1 || i==5))
        plot(freqs,abs(efts(i,:)),"LineWidth",3)
        title("fft Spectrum in the Positive and Negative Frequencies")
        xlabel("f (Hz)")
        ylabel("|fft(X)|")
    end
    hold on;
    % pause
end
hold off;

%%

theta = 0:pi/necho:pi-pi/necho;
pr = transpose(efts);
I2r = iradon(abs(pr),theta*180/pi,'linear','none');
I2i = iradon(imag(pr),theta*180/pi,'linear','none');
% I2 = iradon([abs(efts(1,:)') abs(efts(1,:)')],[0 0]);
% ft = fft(echoes);
% L = size(ft, 2);
% 
% plot(abs(fftshift(ft))
imshow(I2r,[])