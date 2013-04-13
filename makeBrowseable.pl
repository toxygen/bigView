#!/usr/bin/perl -w

# This perl script will take a directory of JPGs
# and convert them to PPM, and the .paged images
# suitable for browsing with the browser program
# Run this script within the directory.
# BUG: this should remove the .ppm's when finished

# IMPORTANT: you'll have to point this to the 
#          : genPaged program. i.e. something like:
$PAGER="/home/sandstro/tmp/bigView/genPaged";

#$PAGER="genPaged";

# generate a PPM thumbnail of a jpeg
sub genThumb {
    my $image = shift(@_);
    my $ppm = $_;
    my $thumb;
    my $command;
    my $width;
    my $height;
    my $max;
    my $fscale;
    my $iscale;
    my $scale;

    $ppm =~ s/JPG/jpg/g;
    $ppm =~ s/\.jpg/.ppm/g;

    $thumb = ".thumbs/".$ppm;
    $thumb =~ s/\.ppm/T.ppm/g;

    $command = "pnmfile $ppm | awk '{print \$4}'";
    $width = `$command`;
    $command = "pnmfile $ppm | awk '{print \$6}'";
    $height = `$command`;
    $max=1.0;
    if( $width>$height){
        $max=$width;
    } else {
        $max=$height;
    }
    $fscale = 256.0/$max;
    $iscale = int $fscale*256;
    $scale = $iscale/256.0;
    # generate a 256x256 thumbnail
    eval(`pnmscale $scale $ppm>$thumb`);
    print "==== genThumb(".$thumb.") ====\n";
}

# create a .thumbs dir is not already existent

if( -e ".thumbs" && -d ".thumbs" ){
} else {
    eval(`/bin/mkdir ./.thumbs`);
}

# find all the jpeg in this dir
open (FILE,"find . -maxdepth 1 \\( -name '*.jpg' -o -name '*.JPG' \\) -print |sort |");

# loop over jpegs, creating thumbs,ppms,and paged files as needed

while ( <FILE> ) {
    # remove newline
    chomp;

    # remove initial './'
    $_ =~ s/^\.\///g;

    $ppm = $_;
    $ppm =~ s/JPG/jpg/g;
    $ppm =~ s/\.jpg/.ppm/g;
    
    $paged = $ppm.".paged";

    if( not -e $paged ){
	eval(`djpeg $_ > $ppm`);
	eval(`$PAGER $ppm`);
	print $paged."\n";
    }

    $thumb = ".thumbs/".$_;
    $thumb =~ s/JPG/jpg/g;
    $thumb =~ s/\.jpg/.ppm/g;
    $thumb =~ s/\.ppm/T.ppm/g;
    if( not -e $thumb ){
        genThumb($_);
    }
}


