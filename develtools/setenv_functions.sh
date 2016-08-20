#!/bin/bash
#
# setenv_functions.sh
#
# Eric Nodwell, October 2015
#
# Provides functions useful for modifying system paths.
#
# You call this from other bach scripts like this:
#
#    source /usr/local/bin/setenv_functions.sh
#
# Then you can make use of the functions defined below.
#  

# Example usage: set_path PATH /usr/my/path
#
function set_path
{
  echo -e "Setting ${1}: \033[1;34m${2}\033[0m"
  eval "$1=\"$2\""
  eval "export $1"
}

# Example usage: delete_path PATH
#
function delete_path
{
  eval "OLD=\"\$$1\""
  if [ -n "${OLD}" ] ; then
    echo -e "Deleting $1"
    eval "unset $1"
  fi
}

# Example usage: add_path PATH /usr/my/path
#
function add_path
{
  echo -e "Adding to ${1}: \033[1;34m${2}\033[0m"
  eval "OLDLIST=\"\$$1\""
  if [ -n "${OLDLIST}" ] ; then
    eval "$1=\"$2:\$$1\""
  else
    eval "$1=\"$2\""
  fi
  eval "export $1"
}
 
# Example usage: remove_path PATH /usr/my/path
#
# The second argument can be a regular expression for pattern matching.
#
function remove_path
{
  eval "OLDLIST=\"\$$1\""
  REMOVALLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -wE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
  if [ -n "${REMOVALLIST}" ] ; then
    echo -e "Removing from ${1}: \033[1;31m${REMOVALLIST}\033[0m"
    NEWLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -vwE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
    eval "$1=\"$NEWLIST\""
    eval "export $1"
  fi
}
 
# Example usage: check_path PATH /usr/my/path
#
# The second argument can be a regular expression for pattern matching.
#
function check_path
{
  eval "OLDLIST=\"\$$1\""
  CHECKLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -wE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
  test -n "${CHECKLIST}"
  return $?
}
# Example usage: add_path PATH /usr/my/path
#
function add_path
{
  echo -e "Adding to ${1}: \033[1;34m${2}\033[0m"
  eval "OLDLIST=\"\$$1\""
  if [ -n "${OLDLIST}" ] ; then
    eval "$1=\"$2:\$$1\""
  else
    eval "$1=\"$2\""
  fi
  eval "export $1"
}
 
# Example usage: remove_path PATH /usr/my/path
#
# The second argument can be a regular expression for pattern matching.
#
function remove_path
{
  eval "OLDLIST=\"\$$1\""
  REMOVALLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -wE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
  if [ -n "${REMOVALLIST}" ] ; then
    echo -e "Removing from ${1}: \033[1;31m${REMOVALLIST}\033[0m"
    NEWLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -vwE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
    eval "$1=\"$NEWLIST\""
    eval "export $1"
  fi
}
 
# Example usage: check_path PATH /usr/my/path
#
# The second argument can be a regular expression for pattern matching.
#
function check_path
{
  eval "OLDLIST=\"\$$1\""
  CHECKLIST=$( echo ${OLDLIST} | tr -s ":" "\n" | grep -wE "${2}" | tr -s "\n" ":" | sed "s/:$//" )
  test -n "${CHECKLIST}"
  return $?
}
