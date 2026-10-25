% Get list of files named "scan*.mat" in current folder
files = dir('fscan*.mat');

% Sort files numerically (so scan3.mat comes before scan10.mat)
[~, idx] = sort( str2double( regexp({files.name}, '\d+', 'match', 'once') ) );
files = files(idx);

% Initialize an empty array
all_scans = [];

% Loop through files and append
for k = 1:numel(files)
    % Load .mat file
    data = load(files(k).name);

    % Extract row 1, all columns
    tmp = data.mm(1, 1:end);

    % Append as new row in the array
    all_scans(k, :) = tmp;
end

ref = all_scans(1, 1:end);

for k = 1:size(all_scans,1)
    all_scans(k,:) = align_signal(all_scans(k,:), ref);
end

%% phase them together

function [aligned, shift] = align_signal(x, ref)
    maxshift = 10;
    [c, lags] = xcorr(x(1:2500), ref(1:2500), maxshift);
    
    % Find the lag with the maximum correlation
    [~, idx] = max(c);
    shift = -lags(idx)
    
    % Apply the shift
    if shift > 0
        aligned = [zeros(1, shift), x(1:end-shift)];
    elseif shift < 0
        aligned = [x((-shift+1):end), zeros(1, -shift)];
    else
        aligned = x;
    end
end


%% demodulation
freq = 4.48*10^6;
for k = 1:size(all_scans,1)
    all_scans(k,:) = demodulation(freq, all_scans(k,:));
end

%% decimation
step = 500;
projs = zeros(size(all_scans,1),38);
echoes = zeros(size(all_scans,1), ...
    length(all_scans(1,165000:200000)));

for k = 1:size(all_scans,1)
    echoes(k,:) = fftshift(fft(all_scans(k,165000:200000)));
    projs(k,:) = echoes(k,17484:17521);
end

%%
% plot(abs(projs(2,:)))
theta = 0:pi/20:19*pi/20;
pr = abs(transpose(projs(2:end,:)));
I2 = iradon(pr,theta*180/pi,'linear','none');
% ft = fft(echoes);
% L = size(ft, 2);
% 
% plot(abs(fftshift(ft))
imshow(I2, [])

%%
plot(pr(:,1))
