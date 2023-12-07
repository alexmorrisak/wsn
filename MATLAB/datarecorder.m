% wsn_serialport.m

s = serialport("COM9", 38400,...
    'Parity', 'even',...
    'DataBits', 7,...
    'StopBits', 1);

%%
fname = sprintf('loradata_%s.csv', datestr(datetime(), 30));
fid = fopen(fname, 'w');
%%
while 1
    data = s.readline();
    disp(data);
    fprintf(fid, "%s\n", data);
end