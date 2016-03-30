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
write.table(avg.deviation, file='output/percentage-deviation.txt', quote=FALSE, col.names=FALSE)


## Compute the fraction of instances that benefits from
## redundancy elimination
algos <- c()
const.benefits <- c()

for (i in c('ch1','ch2','ch3','ch4')) {
	a.file <- paste(i, '.txt', sep='')
	b.file <- paste(i, '+re', '.txt', sep='')
	a.data <- read.table(paste(filepath, 'output', a.file, sep='/'), header=FALSE, row.names=1)
	b.data <- read.table(paste(filepath, 'output', b.file, sep='/'), header=FALSE, row.names=1)

	out.data <- data.frame(row.names=rownames(a.data), benefit=(b.data$V2 < a.data$V2))
	fraction <- sum(out.data$benefit) / nrow(out.data)

	algos <- c(algos, i)
	const.benefits <- c(const.benefits, fraction)
}

const.benefit <- data.frame(row.names=algos, benefits=const.benefits)
write.table(const.benefit, file='output/constructive-benefit.txt', quote=FALSE, col.names=FALSE)

## Compute the amount of benefit from the redundancy elimination
algos <- c()
benefit.mins <- c()
benefit.maxs <- c()
benefit.means <- c()

for (i in c('ch1','ch2','ch3','ch4')) {
	a.file <- paste(i, '.txt', sep='')
	b.file <- paste(i, '+re', '.txt', sep='')
	a.data <- read.table(paste(filepath, 'output', a.file, sep='/'), header=FALSE, row.names=1)
	b.data <- read.table(paste(filepath, 'output', b.file, sep='/'), header=FALSE, row.names=1)

	a.diff <- 100*((a.data$V2 - best$V2)/best$V2)
	b.diff <- 100*((b.data$V2 - best$V2)/best$V2)
	out.data <- a.diff - b.diff
	benefit.min <- min(out.data)
	benefit.max <- max(out.data)
	benefit.mean <- mean(out.data)

	algos <- c(algos, i)
	benefit.mins <- c(benefit.mins, benefit.min)
	benefit.maxs <- c(benefit.maxs, benefit.max)
	benefit.means <- c(benefit.means, benefit.mean)
}

const.benefit <- data.frame(row.names=algos, mins=benefit.mins, maxs=benefit.maxs, means=benefit.means)
write.table(const.benefit, file='output/benefit-data.txt', quote=FALSE, col.names=FALSE)


## Compute the fraction of instances that benefits from
## iterative improvement
algos <- c()
iter.benefits <- c()

for (i in c('ch1', 'ch4')) {
	for (j in c('', '+re')) {
		a.file <- paste(i, j, '+bi', '.txt', sep='')
		b.file <- paste(i, j, '+fi', '.txt', sep='')
		c.file <- paste(i, j, '.txt', sep='')
		
		a.data <- read.table(paste(filepath, 'output', a.file, sep='/'), header=FALSE, row.names=1)
		b.data <- read.table(paste(filepath, 'output', b.file, sep='/'), header=FALSE, row.names=1)
		c.data <- read.table(paste(filepath, 'output', c.file, sep='/'), header=FALSE, row.names=1)

		a.out.data <- data.frame(row.names=rownames(c.data), benefit=(a.data$V2 < c.data$V2))
		b.out.data <- data.frame(row.names=rownames(c.data), benefit=(b.data$V2 < c.data$V2))

		a.fraction <- sum(a.out.data$benefit) / nrow(a.out.data)
		b.fraction <- sum(b.out.data$benefit) / nrow(b.out.data)

		algos <- c(algos, paste(i,j,'+bi',sep=''), paste(i,j,'+fi',sep=''))
		iter.benefits <- c(iter.benefits, a.fraction, b.fraction)
	}
}

iter.benefit <- data.frame(row.names=algos, benefits=iter.benefits)
write.table(iter.benefit, file='output/iterative-benefit.txt', quote=FALSE, col.names=FALSE)

