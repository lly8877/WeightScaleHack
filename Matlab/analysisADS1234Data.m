clear all;
close all;
[channel1, channel2, sum] = ...
    textread('log12.txt','%d\t%d\t%f');
ratio = 29200/85.2;
channel1 = channel1/ratio;
channel2 = channel2/ratio;

delete = 1;
if (delete)
indexToDelete = sum < 60;
channel1(indexToDelete) = [];
channel2(indexToDelete) = [];
sum(indexToDelete) = [];
end

first500 = 1:800;
sum1 = medfilt1(sum,40);
toPlot = [channel1, channel2, sum, sum1];


%toPlot(length(sum1)-500:length(sum1),:) = [];
%toPlot(1:500,:) = [];
plot(toPlot);
std(toPlot(20:120,4))
%ylim([6,7]);
%hist(sum, 500);