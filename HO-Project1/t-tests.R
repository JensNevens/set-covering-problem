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

output <- data.frame(row.names=files)

for (i in files) {
	for (j in files) {
		if (i != j) {
			a.data <- read.table(paste(filepath, 'output', i, sep='/'), header=FALSE, row.names=1)
			b.data <- read.table(paste(filepath, 'output', j, sep='/'), header=FALSE, row.names=1)

			a.cost <- 100*((a.data$V2 - best$V2)/best$V2)
			b.cost <- 100*((b.data$V2 - best$V2)/best$V2)

			p.val <- t.test(a.cost, b.cost, paired=TRUE)$p.value
			output[i,j] = p.val
		}
	}
}

write.table(output, file='output/ttest.txt', quote=FALSE)
