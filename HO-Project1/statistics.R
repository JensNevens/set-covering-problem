##
##
## HO-Project Statistics
## Created by Jens Nevens on 05/03/16.
## Copyright © 2016 Jens Nevens. All rights reserved.
##
##

filepath <- '/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment1/HO-Project1/HO-Project1'

best <- read.table(paste(filepath, 'best.txt', sep="/"), header=FALSE, row.names=1)

files <- list.files(path='./output', pattern='^ch')

## Compute the percentage deviation from the best know solution
## and the average percentage deviation over each algorithm
algos <- c()
avg <- c()

for (i in files) {
	idx <- which(strsplit(i, "")[[1]]==".")
	out.name <- substr(i,0,idx-1)
	algos <- c(algos, out.name)
	out.filename <- paste('percentage', paste(out.name, 'txt', sep='.'), sep="/")
	data <- read.table(paste(filepath, 'output', i, sep="/"), header=FALSE, row.names=1)
	out.data <- data.frame(row.names=rownames(best), perct=100*((data$V2 - best$V2)/best$V2))
	avg <- c(avg, mean(out.data$perct))
	write.table(out.data, file=out.filename, quote=FALSE, col.names=FALSE)
}

avg.deviation <- data.frame(row.names=algos, avg=avg)
write.table(avg.deviation, file='percentage-deviation.txt', quote=FALSE, col.names=FALSE)


## Compute the fraction of instances that benefits from
## redundancy elimination
algos <- c()
const.benefits <- c()

for (i in c('ch1','ch2','ch3','ch4')) {
	file1 <- paste(i, '.txt', sep='')
	file2 <- paste(i, '+re', '.txt', sep='')
	data1 <- read.table(paste(filepath, 'output', file1, sep='/'), header=FALSE, row.names=1)
	data2 <- read.table(paste(filepath, 'output', file2, sep='/'), header=FALSE, row.names=1)

	out.data <- data.frame(row.names=rownames(data1), benefit=(data2$V2 < data1$V2))
	fraction <- sum(out.data$benefit) / nrow(out.data)

	algos <- c(algos, i)
	const.benefits <- c(const.benefits, fraction)
}

const.benefit <- data.frame(row.names=algos, benefits=const.benefits)
write.table(const.benefit, file='constructive-benefit.txt', quote=FALSE, col.names=FALSE)

## Compute the amount of benefit from the redundancy elimination
algos <- c()
benefit.mins <- c()
benefit.maxs <- c()
benefit.means <- c()

for (i in c('ch1','ch2','ch3','ch4')) {
	file1 <- paste(i, '.txt', sep='')
	file2 <- paste(i, '+re', '.txt', sep='')
	data1 <- read.table(paste(filepath, 'output', file1, sep='/'), header=FALSE, row.names=1)
	data2 <- read.table(paste(filepath, 'output', file2, sep='/'), header=FALSE, row.names=1)

	out.data <- data.frame(row.names=rownames(data1), diff=(data1$V2 - data2$V2))
	benefit.min <- min(out.data$diff)
	benefit.max <- max(out.data$diff)
	benefit.mean <- mean(out.data$diff)

	algos <- c(algos, i)
	benefit.mins <- c(benefit.mins, benefit.min)
	benefit.maxs <- c(benefit.maxs, benefit.max)
	benefit.means <- c(benefit.means, benefit.mean)
}

const.benefit <- data.frame(row.names=algos, mins=benefit.mins, maxs=benefit.maxs, means=benefit.means)
write.table(const.benefit, file='benefit-data.txt', quote=FALSE, col.names=FALSE)


## Compute the fraction of instances that benefits from
## iterative improvement
algos <- c()
iter.benefits <- c()

for (i in c('ch1', 'ch4')) {
	for (j in c('', '+re')) {
		file1 <- paste(i, j, '+bi', '.txt', sep='')
		file2 <- paste(i, j, '+fi', '.txt', sep='')
		file3 <- paste(i, j, '.txt', sep='')
		
		data1 <- read.table(paste(filepath, 'output', file1, sep='/'), header=FALSE, row.names=1)
		data2 <- read.table(paste(filepath, 'output', file2, sep='/'), header=FALSE, row.names=1)
		data3 <- read.table(paste(filepath, 'output', file3, sep='/'), header=FALSE, row.names=1)

		out.data1 <- data.frame(row.names=rownames(data3), benefit=(data1$V2 < data3$V2))
		out.data2 <- data.frame(row.names=rownames(data3), benefit=(data2$V2 < data3$V2))

		fraction1 <- sum(out.data1$benefit) / nrow(out.data1)
		fraction2 <- sum(out.data2$benefit) / nrow(out.data2)

		algos <- c(algos, paste(i,j,'+bi',sep=''), paste(i,j,'+fi',sep=''))
		iter.benefits <- c(iter.benefits, fraction1, fraction2)
	}
}

iter.benefit <- data.frame(row.names=algos, benefits=iter.benefits)
write.table(iter.benefit, file='iterative-benefit.txt', quote=FALSE, col.names=FALSE)

