%By Caleb Howell
fileID1 = fopen('xpoints.txt','r');
formatSpec = '%f';
sizexp = [1 Inf];
xp = fscanf(fileID1,formatSpec,sizexp);
fclose(fileID1);

fileID2 = fopen('ypoints.txt','r');
formatSpec = '%f';
sizeyp = [1 Inf];
yp = fscanf(fileID2,formatSpec,sizeyp);
fclose(fileID2);

fileID3 = fopen('zpoints.txt','r');
formatSpec = '%f';
sizezp = [1 Inf];
zp = fscanf(fileID3,formatSpec,sizezp);
fclose(fileID3);

plot3(xp, yp, zp, 'k.');      %k. is to make black dots as points
axis([-15 15 -15 15 -15 15]);