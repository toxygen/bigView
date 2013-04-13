##########################################################################
# makeLinks.perl
# Recursively creates links to top-level Global.defs, RCSMake.defs
# Makes sure localDepend exists
##########################################################################

use File::Basename;

# remove FEL's version so we're all talking the same language
eval(`/bin/rm -fr FEL/Global.defs`);

# get the current directory
$cwd=`pwd`;
chop($cwd);

$topdir = $cwd;

# replace the TOPDIR line in Global.defs
open(INPUT,"Global.defs") or die "opening Global.defs";
open(OUTPUT,"> Global.defs.tmp") or die "opening Global.defs.tmp";

while( <INPUT> )
{
    chomp;
    if( $_ =~ "TOPDIR =" )
    {
	print OUTPUT "TOPDIR = ".$topdir."\n";
	print "Replaced TOPDIR"."\n";
    } 
    else
    {
	print OUTPUT $_."\n";
    }

}
close INPUT;
close OUTPUT;

rename("Global.defs.tmp", "Global.defs");

# show all the files below us
makeLinks($cwd);

# create some directories
eval(`/bin/mkdir lib`);
eval(`/bin/mkdir include`);


sub makeLinks {

    # get the argument passed to us by the client
    my $_dirToList = shift(@_);

    my $_dirName="";

    # force a trailing slash
    if( $_dirToList =~ /\/$/ ){
	$_dirName=$_dirToList;
    } else {
	$_dirName=$_dirToList."/";
    }
    $base = basename($_dirToList);

    if( $base eq "CVS" ){
	return;
    }

    print "===".$_dirName."===\n";

    chdir $_dirToList;

    if( not -e "Global.defs" ){
	eval (`ln -s ../Global.defs Global.defs`);
    }
    if( not -e "localDepend" ){
	eval (`touch localDepend`);
    }

    # get the list of files in this dir
    my @_dirFiles = <$_dirName*>;

    # declare the local
    my $_dirFile="";

    # loop over all the files in this directory
    foreach $_dirFile (@_dirFiles) {
	
	# if this file is a directory, call this sub recursively
	if( -d $_dirFile and not $_dirFile =~ "plink" ){
	    makeLinks($_dirFile);
	    chdir $_dirToList;
	} 
    }
}

