#

# TOMBO Build program for ActivePerl(Win32)

##########################################################################
# Data definition
##########################################################################

$version = "Tombo1_3";
$buildroot = "R:\\work2\\TOMBO";
$builddir  = "R:\\temp\\Tombo-Rel";

$be500root = "R:\\work2\\BE500SDK\\Installer\\Japanese PC Tools";


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

	["", "$buildroot\\Docs\\COPYING-2_0.txt"],
	["", "$buildroot\\Docs\\Readme.txt"],
	["Docs", "$buildroot\\Docs\\HTMLDoc\\*.html"],
	["Docs\\image", "$buildroot\\Docs\\HTMLDoc\\image\\*.bmp"],
);

@buildlistWin32JP = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],

	["", "$buildroot\\Docs\\COPYING-2_0.txt"],
	["", "$buildroot\\Docs\\Readme.txt"],
	["Docs", "$buildroot\\Docs\\HTMLDoc\\*.html"],
	["Docs\\image", "$buildroot\\Docs\\HTMLDoc\\image\\*.bmp"],

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

	["", "$buildroot\\Docs\\COPYING-2_0.txt"],
	["", "$buildroot\\Docs\\Readme_E.txt"],
);

@buildlistWin32EN = (
	["", "$buildroot\\Win32\\Tombo\\ReleaseE\\Tombo.exe"],

	["", "$buildroot\\Docs\\COPYING-2_0.txt"],
	["", "$buildroot\\Docs\\Readme.txt"],
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
		foreach $f (glob($src)) {
			die "$f not exist!" unless (-f "$f");
			print "$f -> $buildto\\$dst\n";
			copy("$f", "$buildto\\$dst\\") || die "$f : $!";
			$num++;
		}
	}
	print "Copy $num files.\n";
}
