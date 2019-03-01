#' Read a single FCS file in to a cytoframe
#' 
#' Similar to \code{\link[flowCore]{read.FCS}}, this takes a filename for a single
#' FCS file and returns a \code{cytoframe}.
#'
#' The function \code{load_cytoframe_from_fcs} works with the output of the FACS machine
#' software from a number of vendors (FCS 2.0, FCS 3.0 and List Mode Data LMD).
#' However, the FCS 3.0 standard includes some options that are not yet
#' implemented in this function. If you need extensions, please let me know.
#' The output of the function is an object of class \code{cytoframe}.
#' 
#' For specifications of FCS 3.0 see \url{http://www.isac-net.org} and the file
#' \url{../doc/fcs3.html} in the \code{doc} directory of the package.
#' 
#' The \code{which.lines} arguments allow you to read a subset of the record as
#' you might not want to read the thousands of events recorded in the FCS file.
#' It is mainly used when there is not enough memory to read one single FCS
#' (which probably will not happen).  It will probably take more time than
#' reading the entire FCS (due to the multiple disk IO).
#'
#'
#' @param files A vector of filenames
#' @param filename Character of length 1: filename
#' @param transformation An character string that defines the type of
#' transformation. Valid values are \code{linearize} (default),
#' \code{linearize-with-PnG-scaling}, or \code{scale}.  The \code{linearize}
#' transformation applies the appropriate power transform to the data. The
#' \code{linearize-with-PnG-scaling} transformation applies the appropriate
#' power transform for parameters stored on log scale, and also a linear
#' scaling transformation based on the 'gain' (FCS \$PnG keywords) for
#' parameters stored on a linear scale. The \code{scale} transformation scales
#' all columns to $[0,10^decades]$.  defaulting to decades=0 as in the FCS4
#' specification.  A logical can also be used: \code{TRUE} is equal to
#' \code{linearize} and \code{FALSE}(or \code{NULL}) corresponds to no
#' transformation.  Also when the transformation keyword of the FCS header is
#' set to "custom" or "applied", no transformation will be used.
#' @param which.lines Numeric vector to specify the indices of the lines to be
#' read. If NULL all the records are read, if of length 1, a random sample of
#' the size indicated by \code{which.lines} is read in.
#' @param alter.names boolean indicating whether or not we should rename the
#' columns to valid R names using \code{\link{make.names}}. The default is
#' FALSE.
#' @param column.pattern An optional regular expression defining parameters we
#' should keep when loading the file. The default is NULL.
#' @param invert.pattern logical. By default, \code{FALSE}. If \code{TRUE},
#' inverts the regular expression specified in \code{column.pattern}. This is
#' useful for indicating the channel names that we do not want to read. If
#' \code{column.pattern} is set to \code{NULL}, this argument is ignored.
#' @param decades When scaling is activated, the number of decades to use for
#' the output.
#' @param is_h5 Boolean indicating whether the data should be stored in h5 format
#' @param h5_filename String specifying a name for the h5 file if \code{is_h5} is TRUE
#' @param min.limit The minimum value in the data range that is allowed. Some
#' instruments produce extreme artifactual values. The positive data range for
#' each parameter is completely defined by the measurement range of the
#' instrument and all larger values are set to this threshold. The lower data
#' boundary is not that well defined, since compensation might shift some
#' values below the original measurement range of the instrument. This can be 
#' set to an arbitrary number or to \code{NULL} (the default value), in which 
#' case the original values are kept.
#' @param truncate_max_range logical type. Default is TRUE. can be optionally
#' turned off to avoid truncating the extreme positive value to the instrument
#' measurement range .i.e.'$PnR'.
#' @param dataset The FCS file specification allows for multiple data segments
#' in a single file. Since the output of \code{load_cytoframe_from_cytoset} is a single
#' \code{cytoframe} we can't automatically read in all available sets. This
#' parameter allows to chose one of the subsets for import. Its value is
#' supposed to be an integer in the range of available data sets. This argument
#' is ignored if there is only a single data segment in the FCS file.
#' @param emptyValue Boolean indicating whether or not we allow empty value for
#' keyword values in TEXT segment.  It affects how the double delimiters are
#' treated.  IF TRUE, The double delimiters are parsed as a pair of start and
#' end single delimiter for an empty value.  Otherwise, double delimiters are
#' parsed one part of string as the keyword value.  default is TRUE.
#' @param num_threads Integer allowing for parallelization of the parsing
#' operation by specifiying a number of threads 
#' @param ignore.text.offset Whether to ignore the keyword values in TEXT
#' segment when they don't agree with the HEADER.  Default is FALSE, which
#' throws the error when such discrepancy is found.  User can turn it on to
#' ignore TEXT segment when he is sure of the accuracy of HEADER so that the
#' file still can be read.
#' @param \dots Further arguments that get passed on to
#' \code{\link[Biobase]{read.AnnotatedDataFrame}}, see details
#' 
#' @return
#' An object of class
#' \code{\link{cytoframe}} that contains the data, the parameters monitored, 
#' and the keywords and values saved in the header of the FCS file.
#' 
#' @name load_cytoframe_from_fcs
#' @family cytoframe/cytoset IO functions
#' @export
load_cytoframe_from_fcs <- function(filename,
                     transformation="linearize",
                     which.lines=NULL,
                     alter.names=FALSE,
                     column.pattern=NULL,
                     invert.pattern = FALSE,
                     decades=0,
					 is_h5=FALSE,
					 h5_filename = tempfile(fileext = ".h5"),
                     min.limit=NULL,
                     truncate_max_range = TRUE,
                     dataset=NULL,
                     emptyValue=TRUE,
                     num_threads = 1,
                     ignore.text.offset = FALSE,
                     text.only = FALSE)
{
    fr <- new("cytoframe")
    if(is.null(dataset))
      dataset <- 1
    if(is.null(min.limit)){
      truncate_min_val <- FALSE
      min.limit <- -111
    }else
      truncate_min_val <- TRUE
    if(is.null(which.lines))
      which.lines <- vector()
    else
	{
		# Verify that which.lines is positive and within file limit.
		if (length(which.lines) > 1) {
			which.lines <- which.lines -1
		}
	}
    fr@pointer <- parseFCS(normalizePath(filename), list(which.lines = which.lines
                                                         , transformation = transformation
                                                         , decades = decades
                                                         , truncate_min_val = truncate_min_val
                                                         , min_limit = min.limit
                                                         , truncate_max_range = truncate_max_range
                                                         , dataset = dataset
                                                         , emptyValue = emptyValue
                                                         , num_threads = num_threads
                                                         , ignoreTextOffset = ignore.text.offset
                                                         )
                                                     , text_only = text.only
											 		 , is_h5 = is_h5
											 		 , h5_filename = h5_filename
                            )
     fr@use.exprs <- !text.only

    return(fr)
}

#' Read one or several FCS files in to a cytoset
#' 
#' Similar to \code{\link[flowCore]{read.flowSet}}, this takes a list of FCS filenames
#' and returns a \code{cytoset}.
#' 
#' There are four different ways to specify the file from which data is to be
#' imported:
#' 
#' First, if the argument \code{phenoData} is present and is of class
#' \code{\link[Biobase:class.AnnotatedDataFrame]{AnnotatedDataFrame}}, then the
#' file names are obtained from its sample names (i.e. row names of the
#' underlying data.frame).  Also column \code{name} will be generated based on
#' sample names if it is not there. This column is mainly used by visualization
#' methods in flowViz.  Alternatively, the argument \code{phenoData} can be of
#' class \code{character}, in which case this function tries to read a
#' \code{AnnotatedDataFrame} object from the file with that name by calling
#' \code{\link[Biobase]{read.AnnotatedDataFrame}(file.path(path,phenoData),\dots{})}.
#' 
#' In some cases the file names are not a reasonable selection criterion and
#' the user might want to import files based on some keywords within the file.
#' One or several keyword value pairs can be given as the phenoData argument in
#' form of a named list.
#' 
#' Third, if the argument \code{phenoData} is not present and the argument
#' \code{files} is not \code{NULL}, then \code{files} is expected to be a
#' character vector with the file names.
#' 
#' Fourth, if neither the argument \code{phenoData} is present nor \code{files}
#' is not \code{NULL}, then the file names are obtained by calling
#' \code{dir(path, pattern)}.
#' @param files Optional character vector with filenames.
#' @param path Directory where to look for the files.
#' @param pattern This argument is passed on to
#' \code{\link[base:list.files]{dir}}, see details.
#' @param phenoData An object of class \code{AnnotatedDataFrame},
#' \code{character} or a list of values to be extracted from the
#' \code{\link{cytoframe}} object, see details.
#' @param descriptions Character vector to annotate the object of class
#' \code{\link{cytoset}}.
#' @param name.keyword An optional character vector that specifies which FCS
#' keyword to use as the sample names. If this is not set, the GUID of the FCS
#' file is used for sampleNames, and if that is not present (or not unique),
#' then the file names are used.
#' @param transformation see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param which.lines see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param alter.names see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param column.pattern see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param invert.pattern see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param decades see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param is_h5 Boolean indicating whether the data should be stored in h5 format
#' @param min.limit see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param truncate_max_range see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param dataset see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param emptyValue see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param num_threads Integer allowing for parallelization of the parsing
#' operation by specifiying a number of threads
#' @param ignore.text.offset see \code{\link{load_cytoframe_from_fcs}} for details.
#' @param sep Separator character that gets passed on to
#' \code{\link[Biobase]{read.AnnotatedDataFrame}}.
#' @param as.is Logical that gets passed on to
#' \code{\link[Biobase]{read.AnnotatedDataFrame}}. This controls the automatic
#' coercion of characters to factors in the \code{phenoData}.
#' @param name An optional character scalar used as name of the object.
#' @param h5_dir String specifying a name for the h5 directory
#' for the h5 files if \code{is_h5} is TRUE
#' @param \dots Further arguments that get passed on to
#' \code{\link[Biobase]{read.AnnotatedDataFrame}}, see details.
#' 
#' @return An object of class \code{\link{cytoset}}.
#' 
#' @name load_cytoset_from_fcs
#' @family cytoframe/cytoset IO functions
#' @export
#' @importFrom Biobase read.AnnotatedDataFrame
load_cytoset_from_fcs <- function(files=NULL, path=".", pattern=NULL, phenoData,
                         descriptions, name.keyword,
                         transformation="linearize",
                         which.lines=NULL,
                         alter.names=FALSE,
                         column.pattern=NULL,
                         invert.pattern = FALSE,
                         decades=0,
                         is_h5=FALSE,
                         min.limit=NULL,
                         truncate_max_range = TRUE,
                         dataset=NULL,
                         emptyValue=TRUE,
                         num_threads = 1,
                         ignore.text.offset = FALSE,
                         sep="\t", as.is=TRUE, name
                        , h5_dir = tempdir()
                         , ...)
{
    ## A frame of phenoData information
    phenoFrame <- NULL
    if(!dir.exists(h5_dir))
      dir.create(h5_dir)
    ## deal with the case that the phenoData is provided, either as
    ## character vector or as AnnotatedDataFrame.
    if(!missing(phenoData)) {
        if(is.character(phenoData) && length(phenoData) == 1){
            phenoData <- read.AnnotatedDataFrame(file.path(path, phenoData),
                                                 header = TRUE, sep=sep
                                                 , as.is=as.is
                                                 , colClasses = c(FCS_File = "character") #avoid coersing filename to numbers that  accidentally tampers the filename by stripping leading zeros
                                                 , ...)
            ## the sampleNames of the Annotated data frame must match the
            ## file names and we try to guess them from the input
            fnams <- grep("file|filename", varLabels(phenoData),
                          ignore.case=TRUE)
            if(length(fnams)){
                fn <- as.character(unlist(pData(phenoData[,fnams[1]])))
                if(any(duplicated(fn)))
                    stop("The file names supplied as part of the ",
                         "phenoData are not unique", call.=FALSE)
                sampleNames(phenoData) <- fn
                pd <- pData(phenoData)
                pd[,fnams[1]] <- fn
                pData(phenoData) <- pd
            }
            phenoFrame <- phenoData
        }else if(is(phenoData,"AnnotatedDataFrame")){
            phenoFrame <- phenoData
        }else{if(!is.list(phenoData))
                  stop("Argument 'phenoData' must be of type 'list', ",
                       "'AnnotatedDataFrame' or a filename\n",
                       "of a text file containing the phenotypic information")
          }
    }

    ## go on and find the files
    if(!is.null(phenoFrame)) {
        if(!is.null(files))
            warning("Supplied file names will be ignored, ",
                    "using names in the phenoData slot instead.")
        file.names <- sampleNames(phenoFrame)
	    files <- file.path(path, file.names)
      	if(!all(file.exists(files)))
            stop(paste("Not all files given by phenoData could be found in",
                       path))
        if(!"name" %in% varLabels(phenoFrame)){
            phenoFrame$name <- basename(files)
            varMetadata(phenoFrame)["name",] <- "Filename"
        }
    }else{
        ## if we haven't found files by now try to search according to
        ## 'pattern'
        if(is.null(files)) {
            files <- dir(path,pattern,full.names=TRUE)
            file.names <- dir(path,pattern,full.names=FALSE)
            if(length(files)<1)
                stop(paste("No matching files found in ",path))
        } else {
            if(!is.character(files))
                stop("'files' must be a character vector.")
            file.names <- basename(files) ## strip path from names
            if(path != ".")
                files <- file.path(path, files)
        }
    }
  
    if(is.null(dataset))
      dataset <- 1
    if(is.null(min.limit)){
      truncate_min_val <- FALSE
      min.limit <- -111
    }else
      truncate_min_val <- TRUE
    if(is.null(which.lines))
      which.lines <- vector()
    else
      which.lines <- which.lines -1
    guids <- make.unique(file.names)
    names(files) <- guids
    cs <- fcs_to_cytoset(files, list(which_lines = which.lines
                                            , transformation = transformation
                                            , decades = decades
                                            , truncate_min_val = truncate_min_val
                                            , min_limit = min.limit
                                            , truncate_max_range = truncate_max_range
                                            , dataset = dataset
                                            , emptyValue = emptyValue
                                            , num_threads = num_threads
                                            , ignoreTextOffset = ignore.text.offset
                                          )
                                          , is_h5 = is_h5
                                          , h5_dir = normalizePath(h5_dir)
                                  )
    cs <- new("cytoset", pointer = cs)
    
    if(!is.null(phenoFrame))
    {
      pd <- pData(phenoFrame)
      pd[["FCS_File"]] <- NULL
      pData(cs) <- pd
    }
    
    cs
    
}
