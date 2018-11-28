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
                     fast = TRUE,
                     num_threads = 1,
                     ignore.text.offset = FALSE,
                     text.only = FALSE)
{
    fr <- new("cytoFrame")
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
    fr@pointer <- parseFCS(normalizePath(filename), list(which_lines = which.lines
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
                         fast = TRUE,
                         num_threads = 1,
                         ignore.text.offset = FALSE,
                         sep="\t", as.is=TRUE, name
                        , h5_dir = tempfile()
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
    cs <- new("cytoSet", pointer = cs)
    
    if(!is.null(phenoFrame))
    {
      pd <- pData(phenoFrame)
      pd[["FCS_File"]] <- NULL
      pData(cs) <- pd
    }
    
    cs
    
}
