##
##
## HO-Project Statistics
## Created by Jens Nevens on 22/03/16.
## Copyright © 2016 Jens Nevens. All rights reserved.
##
##

filepath <- '/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment1/HO-Project1/HO-Project1'

best <- read.table(paste(filepath, 'best.txt', sep="/"), header=FALSE, row.names=1)

files <- list.files(path='./output', pattern='^ch')


## Apply t-test between each pair of algorithms
## This is applied on percentage deviation from
## best known solution
for (i in c("ch1", "ch2", "ch3", "ch4")) {
	for (j in c("ch1", "ch2", "ch3", "ch4")) {
		if (i != j) {
			a.file <- paste(i, '.txt', sep='')
			b.file <- paste(j, '.txt', sep='')

			a.data <- read.table(paste(filepath, 'output', a.file, sep='/'), header=FALSE, row.names=1)
			b.data <- read.table(paste(filepath, 'output', b.file, sep='/'), header=FALSE, row.names=1)

			a.cost <- 100*((a.data$V2 - best$V2)/best$V2)
			b.cost <- 100*((b.data$V2 - best$V2)/best$V2)

			p.val <- t.test(a.cost, b.cost, paired=T)$p.value
			print(i)
			print(j)
			print(p.val)
		}
	}
}

## Apply t-test on each algorithm with and
## without RE. This is done on the percentage
## deviation from the best known solution
for (i in c("ch1", "ch2", "ch3", "ch4")) {
	a.file <- paste(i, '.txt', sep='')
	b.file <- paste(i, '+re', '.txt', sep='')

	a.data <- read.table(paste(filepath, 'output', a.file, sep='/'), header=FALSE, row.names=1)
	b.data <- read.table(paste(filepath, 'output', b.file, sep='/'), header=FALSE, row.names=1)

	a.cost <- 100*((a.data$V2 - best$V2)/best$V2)
	b.cost <- 100*((b.data$V2 - best$V2)/best$V2)

	p.val <- t.test(a.cost, b.cost, paired=T)$p.value
	print(i)
	print(p.val)
}







