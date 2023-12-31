% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cytoframe.R
\name{cf_append_cols}
\alias{cf_append_cols}
\title{Append data columns to a flowFrame}
\usage{
cf_append_cols(cf, cols)
}
\arguments{
\item{cf}{A \code{cytoframe}.}

\item{cols}{A numeric matrix containing the new data columns to be added.
Must has column names to be used as new channel names.}
}
\description{
Append data columns to a flowFrame
}
\details{
It is used to add extra data columns to the existing flowFrame.  It handles
keywords and parameters properly to ensure the new flowFrame can be written
as a valid FCS through the function \code{write.FCS} .
}
\examples{

  library(flowCore)
  data(GvHD)
  tmp <- GvHD[[1]]
  cf <- flowFrame_to_cytoframe(tmp)
  kf <- kmeansFilter("FSC-H"=c("Pop1","Pop2","Pop3"), filterId="myKmFilter")
  fres <- filter(cf, kf)
  cols <- as.numeric(fres@subSet)
  cols <- matrix(cols, dimnames = list(NULL, "km"))
  cf <- cf_append_cols(cf, cols)
  


}
