#

$version = "Tombo1_6b2";

use File::Glob ':glob';

# TOMBO Build program for ActivePerl(Win32)

##########################################################################
# Data definition
##########################################################################

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

	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],

	["Bin\\BE500", "$be500root\\Setup.exe"],
	["Bin\\BE500", "$be500root\\Setup.ini"],
	["Bin\\BE500", "$be500root\\Tombo.cbea"],


	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3Rel\\Tombo.exe"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\BE500", "$buildroot\\BE500\\Tombo\\MIPSRel\\Tombo.exe"],


	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
);

@buildlistWin32JP = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
);

@buildlist_HPC_JP = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
);

@buildlist_PKTPC_JP = (
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\Tombo.sh3.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3Rel\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
);

@buildlist_PSPC_JP = (
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
);

@buildlist_BE500_JP = (
	["Bin\\BE500", "$be500root\\Setup.exe"],
	["Bin\\BE500", "$be500root\\Setup.ini"],
	["Bin\\BE500", "$be500root\\Tombo.cbea"],
	["Bin\\BE500", "$buildroot\\BE500\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
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

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],

	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],

	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

@buildlistWin32EN = (
	["", "$buildroot\\Win32\\Tombo\\ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

@buildlist_HPC_EN = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3ReleaseE\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

@buildlist_PKTPC_EN = (
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\TomboE.sh3.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

@buildlist_PSPC_EN = (
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSReleaseE\\Tombo.exe"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\TomboE.sh3.CAB"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

@buildlist_BE300_EN = (
	["Bin\\BE300", "$be300root\\Setup.exe"],
	["Bin\\BE300", "$be300root\\Setup.ini"],
	["Bin\\BE300", "$be300root\\Tombo.cbea"],
	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
);

##########################################################################
# Tool definition
##########################################################################

$zippg = "\"C:\\Program Files\\mzp\\mzp.exe\"";

##########################################################################
# Program section.
##########################################################################

use File::Path;
use File::Copy;
use File::DosGlob 'glob';

&cpfile("$builddir\\$version\\CE-JP\\$version", \@buildlistCEJP);
&cpfile("$builddir\\$version\\CE-EN\\$version", \@buildlistCEEN);
&cpfile("$builddir\\$version\\Win32-JP\\$version", \@buildlistWin32JP);
&cpfile("$builddir\\$version\\Win32-EN\\$version", \@buildlistWin32EN);

&cpfile("$builddir\\$version\\HPC-JP\\$version",  \@buildlist_HPC_JP);
&cpfile("$builddir\\$version\\PKTPC-JP\\$version",  \@buildlist_PKTPC_JP);
&cpfile("$builddir\\$version\\PSPC-JP\\$version", \@buildlist_PSPC_JP);
&cpfile("$builddir\\$version\\BE500-JP\\$version",  \@buildlist_BE500_JP);

&cpfile("$builddir\\$version\\HPC-EN\\$version",  \@buildlist_HPC_EN);
&cpfile("$builddir\\$version\\PKTPC-EN\\$version",  \@buildlist_PKTPC_EN);
&cpfile("$builddir\\$version\\PSPC-EN\\$version", \@buildlist_PSPC_EN);
&cpfile("$builddir\\$version\\BE300-EN\\$version",  \@buildlist_BE300_EN);

# make zip file 

&zipfile("$builddir\\$version\\CE-JP", "_ce_all_jp", $version);
&zipfile("$builddir\\$version\\CE-EN", "_ce_all_en", $version);
&zipfile("$builddir\\$version\\Win32-JP", "_win32_jp", $version);
&zipfile("$builddir\\$version\\Win32-En", "_win32_en", $version);

&zipfile("$builddir\\$version\\HPC-EN", "_hpcpro_en", $version);
&zipfile("$builddir\\$version\\PKTPC-EN", "_pktpc_en", $version);
&zipfile("$builddir\\$version\\PSPC-EN", "_pspc_en", $version);
&zipfile("$builddir\\$version\\BE300-EN", "_be300_en", $version);

&zipfile("$builddir\\$version\\HPC-JP", "_hpcpro_jp", $version);
&zipfile("$builddir\\$version\\PKTPC-JP", "_pktpc_jp", $version);
&zipfile("$builddir\\$version\\PSPC-JP", "_pspc_jp", $version);
&zipfile("$builddir\\$version\\BE500-JP", "_be500_jp", $version);

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

sub zipfile {
	my ($dir, $suffix, $target) = @_;
	chdir($dir);
	system("$zippg -r ..\\$version$suffix.zip $version");
}
