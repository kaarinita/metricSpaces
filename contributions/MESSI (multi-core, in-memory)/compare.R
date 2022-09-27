#!/usr/bin/env Rscript


d1 <- read.csv(commandArgs(TRUE)[1], header=T, sep='\t')
d2 <- read.csv(commandArgs(TRUE)[2], header=T, sep='\t')

pdf("comparison.pdf")

for(n in names(d1)) {

    d1_col <- c(d1[n])
    d2_col <- c(d2[n])
    d1_col <- unlist(d1_col)
    d2_col <- unlist(d2_col)

    #d1_col <- c(d1_col[1], diff(d1_col))
    #d2_col <- c(d2_col[1], diff(d2_col))
    m <- max(d1_col, d2_col)

    plot(d1_col, type='l', ylim=c(0,m), col='red', xlab="Query #", ylab=n); 
    lines(d2_col, type='l', col='green');

    legend(0,m - m/1000,c(commandArgs(TRUE)[1:2]), pch = c(0,0), col=c("red", "green"))

}
dev.off()
