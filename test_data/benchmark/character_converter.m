load('mixoutALL_shifted.mat');
for i = 1:2858
	% export .txt file
	A = cell2mat(mixout(1,i));
	A(3,:) = [];
	A = A';
	l = length(A);
	B = tril(ones(l,l), -1)*A;
	save(sprintf('data/%d.txt', i),'B','-ascii');
	% export .jpg file
	% f = figure('visible','off');
	% plot(B(:,1),B(:,2));
	% saveas(f, sprintf('data/%d', i), 'jpg');
end
