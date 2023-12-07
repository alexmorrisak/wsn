% plot_data.m

fname = uigetfile('*.csv');
d = csvread(fname);
figure
plot(d(:,1), d(:,2));