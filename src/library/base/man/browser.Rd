% File src/library/base/man/browser.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2011 R Core Team
% Distributed under GPL 2 or later

\name{browser}
\alias{browser}
\title{Environment Browser}
\description{
  Interrupt the execution of an expression and allow the inspection of
  the environment where \code{browser} was called from.
}
\usage{
browser(text = "", condition = NULL, expr = TRUE, skipCalls = 0L)
}
\arguments{
  \item{text}{a text string that can be retrieved once the browser is invoked.}
  \item{condition}{a condition that can be retrieved once the browser is
    invoked.}
  \item{expr}{An expression, which if it evaluates to \code{TRUE} the
    debugger will invoked, otherwise control is returned directly.}
  \item{skipCalls}{how many previous calls to skip when reporting the
    calling context.}
}
\details{
  A call to \code{browser} can be included in the body of a function.
  When reached, this causes a pause in the execution of the
  current expression and allows access to the \R interpreter.

  The purpose of the \code{text} and \code{condition} arguments are to
  allow helper programs (e.g. external debuggers) to insert specific
  values here, so that the specific call to browser (perhaps its location
  in a source file) can be identified and special processing can be
  achieved.  The values can be retrieved by calling \code{\link{browserText}}
  and \code{\link{browserCondition}}.

  The purpose of the \code{expr} argument is to allow for the illusion
  of conditional debugging. It is an illusion, because execution is
  always paused at the call to browser, but control is only passed
  to the evaluator described below if \code{expr} evaluates to \code{TRUE}.
  In most cases it is going to be more efficient to use an \code{if}
  statement in the calling program, but in some cases using this argument
  will be simpler.

  The \code{skipCalls} argument should be used when the \code{browser()}
  call is nested within another debugging function:  it will look further
  up the call stack to report its location.

  At the browser prompt the user can enter commands or \R expressions,
  followed by a newline.  The commands are

  \describe{
    \item{\code{c}}{(or just an empty line, by default) exit the browser
    and continue execution at the next statement.}
    \item{\code{cont}}{synonym for \code{c}.}
    \item{\code{n}}{enter the step-through debugger if the function is
      interpreted.  This changes the meaning of \code{c}: see the
      documentation for \code{\link{debug}}. For byte compiled functions
      \code{n} is equivalent to \code{c}.}
    \item{\code{where}}{print a stack trace of all active function calls.}
    \item{\code{Q}}{exit the browser and the current evaluation and
      return to the top-level prompt.}
  }
  (Leading and trailing whitespace is ignored, except for an empty line).

  Anything else entered at the browser prompt is interpreted as an
  \R expression to be evaluated in the calling environment: in
  particular typing an object name will cause the object to be printed,
  and \code{ls()} lists the objects in the calling frame.  (If you want
  to look at an object with a name such as \code{n}, print it
  explicitly.)

  The number of lines printed for the deparsed call can be limited by
  setting \code{\link{options}(deparse.max.lines)}.

  Setting \link[=options]{option} \code{"browserNLdisabled"} to \code{TRUE}
  disables the use of an empty line as a synonym for \code{c}.  If this
  is done, the user will be re-prompted for input until a valid command
  or an expression is entered.

  This is a primitive function but does argument matching in the
  standard way.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth & Brooks/Cole.

  Chambers, J. M. (1998)
  \emph{Programming with Data. A Guide to the S Language}.
  Springer.
}
\seealso{
  \code{\link{debug}}, and
  \code{\link{traceback}} for the stack on error.
  \code{\link{browserText}} for how to retrieve the text and condition.
}
\keyword{programming}
\keyword{environment}
