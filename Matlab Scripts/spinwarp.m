% extract the projections to use
freq = 4.52e6;
echos = zeros(60,59,'like', 1j);

%%
for i = 1:59
    disp(i)
    filename = 'te1/te1_' + string(i) + '.mat';
    acq = load(filename).mm;
    echo = demodulation(freq, acq(1,:));
    echos(:,i) = echo(316+22:418-21);
    % echos(:,i) = echo(316+22+4000:418-21+4000);
    % plot(real(echos(:,i)))
    % hold on;
    % plot(imag(echos(:,i)))
    % hold off;
end

%%
%calculate FT
img1 = fftshift(fft2(fftshift(echos,1)));
figure(2);
subplot(1,2,1);
imagesc([-1.33/2 1.33/2],[-1.33/2 1.33/2],abs(echos));
colormap('gray');
axis square;
xlabel('kz (mm^{-1})');
ylabel('ky (mm^{-1})');
title('k Space');
axis on;
subplot(1,2,2);
imagesc([-22.5,22.5],[-22.5,22.5],abs(img1));
colormap('gray');
axis square;
xlabel('Z (mm)');
ylabel('Y (mm)');
title('Image');
