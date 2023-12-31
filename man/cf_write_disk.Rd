% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cytoframe.R
\name{cf_write_disk}
\alias{cf_write_disk}
\title{Save the cytoframe to disk}
\usage{
cf_write_disk(cf, filename, backend = get_default_backend())
}
\arguments{
\item{cf}{cytoframe object}

\item{filename}{the full path of the output file}

\item{backend}{either "h5" or "tile"}
}
\description{
Save the cytoframe to disk
}
\seealso{
Other cytoframe/cytoset IO functions: 
\code{\link{cf_get_uri}()},
\code{\link{cf_write_h5}()},
\code{\link{cs_get_uri}()},
\code{\link{load_cytoframe_from_fcs}()},
\code{\link{load_cytoframe}()},
\code{\link{load_cytoset_from_fcs}()}
}
\concept{cytoframe/cytoset IO functions}
