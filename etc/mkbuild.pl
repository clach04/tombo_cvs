#

use File::Glob ':glob';

# TOMBO Build program for ActivePerl(Win32)

##########################################################################
# Data definition
##########################################################################

$version = "Tombo1_5";
$buildroot = "R:\\work2\\SF\\TOMBO";
$docroot = "R:\\work2\\SF\\Docs";
$builddir  = "R:\\temp\\Tombo-Rel";

$be500root = "R:\\work2\\BE500SDK\\Installer\\Japanese PC Tools";
$be300root = "R:\\work2\\BE300SDK_1012\\Installer\\English PC Tools";


@buildlistCEJP = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\Tombo.sh3.CAB"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],

	["Bin\\BE500", "$be500root\\Setup.exe"],
	["Bin\\BE500", "$be500root\\Setup.ini"],
	["Bin\\BE500", "$be500root\\Tombo.cbea"],
	["Bin\\BE500", "$buildroot\\BE500\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlistWin32JP = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlistCEEN = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3ReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4ReleaseE\\Tombo.exe"],

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\TomboE.sh3.CAB"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\TomboE.sh3.CAB"],

	["Bin\\BE300", "$be300root\\Setup.exe"],
	["Bin\\BE300", "$be300root\\Setup.ini"],
	["Bin\\BE300", "$be300root\\Tombo.cbea"],
	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],

);

@buildlistWin32EN = (
	["", "$buildroot\\Win32\\Tombo\\ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

##########################################################################
# Program section.
##########################################################################

use File::Path;
use File::Copy;
use File::DosGlob 'glob';

&cpfile("$builddir\\JP-CE\\$version", \@buildlistCEJP);
&cpfile("$builddir\\EN-CE\\$version", \@buildlistCEEN);
&cpfile("$builddir\\JP-Win32\\$version", \@buildlistWin32JP);
&cpfile("$builddir\\EN-Win32\\$version", \@buildlistWin32EN);

sub cpfile {
	my ($buildto, $list) = @_;
	my ($i, $num);

	$num = 0;

	foreach $i (@{$list}) {
		($dst, $src) = @{$i};

		# Create directory if not exist.
		mkpath("$buildto\\$dst") || die "mkdir $buildto\\$dst : $!" unless (-d "$buildto\\$dst");

		# Check file existance and copy it.
		foreach $f (bsd_glob($src)) {
			die "$f not exist!" unless (-f "$f");
			print "$f -> $buildto\\$dst\n";
			copy("$f", "$buildto\\$dst\\") || die "$f : $!";
			$num++;
		}
	}
	print "Copy $num files.\n";
}
