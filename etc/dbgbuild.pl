#

use File::Glob ':glob';

# TOMBO Build program for ActivePerl(Win32)

##########################################################################
# Data definition
##########################################################################

$version = "Tombo_SNAPSHOT_20021218";
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

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3Rel\\Tombo.exe"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSRel\\Tombo.exe"],

	["Bin\\BE500", "$buildroot\\BE500\\Tombo\\MIPSRel\\Tombo.exe"],
);

@buildlistWin32JP = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],
);

@buildlistCEEN = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3ReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4ReleaseE\\Tombo.exe"],

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],

	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],
);

@buildlistWin32EN = (
	["", "$buildroot\\Win32\\Tombo\\ReleaseE\\Tombo.exe"],
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
