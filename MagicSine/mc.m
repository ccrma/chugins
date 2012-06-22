

% magic circle test

freq = 220;
fs = 44100;
t = 5;

epsilon = 2*sin(2*pi*freq/fs/2);

x = 1;
y = 0;

x_1 = 0;
y_1 = 0;

sig = zeros(t*fs, 1);

for n = 1:t*fs
    sig(n) = y;
    x_1 = x;
    y_1 = y;
    
    x = x_1 + epsilon*y_1;
    y = y_1 + -epsilon*x;
end

plot((0:numel(sig)-1)/fs, sig);
