#

$version = "Tombo1_5_1";

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

@buildlist_HPC_ARM_JP = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_MIPS_JP = (
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_SH3_JP = (
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_SH4_JP = (
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_ARM_JP = (
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_MIPS_JP = (
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\Tombo.mips.CAB"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_SH3_JP = (
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\Tombo.sh3.CAB"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_PSPC_SH3_JP = (
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_PSPC_MIPS_JP = (
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],

	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["Docs", "$docroot\\Jp\\Docs\\*.html"],
	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
);

@buildlist_BE500_MIPS_JP = (
	["Bin\\BE500", "$be500root\\Setup.exe"],
	["Bin\\BE500", "$be500root\\Setup.ini"],
	["Bin\\BE500", "$be500root\\Tombo.cbea"],
	["Bin\\BE500", "$buildroot\\BE500\\Tombo\\MIPSRel\\Tombo.exe"],

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

@buildlist_HPC_ARM_EN = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_MIPS_EN = (
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_SH3_EN = (
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_HPC_SH4_EN = (
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4ReleaseE\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_ARM_EN = (
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_MIPS_EN = (
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\TomboE.mips.CAB"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_PKTPC_SH3_EN = (
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\TomboE.sh3.CAB"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_PSPC_MIPS_EN = (
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\TomboE.sh3.CAB"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_PSPC_SH3_EN = (
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\TomboE.mips.CAB"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
);

@buildlist_BE300_MIPS_EN = (
	["Bin\\BE300", "$be300root\\Setup.exe"],
	["Bin\\BE300", "$be300root\\Setup.ini"],
	["Bin\\BE300", "$be300root\\Tombo.cbea"],
	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],

	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["Docs", "$docroot\\En\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
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

&cpfile("$builddir\\CE-JP\\$version", \@buildlistCEJP);
&cpfile("$builddir\\CE-EN\\$version", \@buildlistCEEN);
&cpfile("$builddir\\Win32-JP\\$version", \@buildlistWin32JP);
&cpfile("$builddir\\Win32-EN\\$version", \@buildlistWin32EN);

&cpfile("$builddir\\HPC-ARM-JP\\$version",  \@buildlist_HPC_ARM_JP);
&cpfile("$builddir\\HPC-MIPS-JP\\$version", \@buildlist_HPC_MIPS_JP);
&cpfile("$builddir\\HPC-SH3-JP\\$version", \@buildlist_HPC_SH3_JP);
&cpfile("$builddir\\HPC-SH4-JP\\$version", \@buildlist_HPC_SH4_JP);

&cpfile("$builddir\\PKTPC-ARM-JP\\$version",  \@buildlist_PKTPC_ARM_JP);
&cpfile("$builddir\\PKTPC-MIPS-JP\\$version", \@buildlist_PKTPC_MIPS_JP);
&cpfile("$builddir\\PKTPC-SH3-JP\\$version",  \@buildlist_PKTPC_SH3_JP);

&cpfile("$builddir\\PSPC-MIPS-JP\\$version", \@buildlist_PSPC_MIPS_JP);
&cpfile("$builddir\\PSPC-SH3-JP\\$version",  \@buildlist_PSPC_SH3_JP);

&cpfile("$builddir\\HPC-ARM-EN\\$version",  \@buildlist_HPC_ARM_EN);
&cpfile("$builddir\\HPC-MIPS-EN\\$version", \@buildlist_HPC_MIPS_EN);
&cpfile("$builddir\\HPC-SH3-EN\\$version", \@buildlist_HPC_SH3_EN);
&cpfile("$builddir\\HPC-SH4-EN\\$version", \@buildlist_HPC_SH4_EN);

&cpfile("$builddir\\PKTPC-ARM-EN\\$version",  \@buildlist_PKTPC_ARM_EN);
&cpfile("$builddir\\PKTPC-MIPS-EN\\$version", \@buildlist_PKTPC_MIPS_EN);
&cpfile("$builddir\\PKTPC-SH3-EN\\$version",  \@buildlist_PKTPC_SH3_EN);

&cpfile("$builddir\\PSPC-MIPS-EN\\$version", \@buildlist_PSPC_MIPS_EN);
&cpfile("$builddir\\PSPC-SH3-EN\\$version",  \@buildlist_PSPC_SH3_EN);

&cpfile("$builddir\\BE300-MIPS-EN\\$version",  \@buildlist_BE300_MIPS_EN);
&cpfile("$builddir\\BE500-MIPS-JP\\$version",  \@buildlist_BE500_MIPS_JP);

&zipfile("$builddir\\CE-JP", "_ce_jp", $version);
&zipfile("$builddir\\CE-EN", "_ce_en", $version);
&zipfile("$builddir\\Win32-JP", "_win32_jp", $version);
&zipfile("$builddir\\Win32-En", "_win32_en", $version);

&zipfile("$builddir\\HPC-ARM-EN", "_hpcpro_arm_en", $version);
&zipfile("$builddir\\HPC-MIPS-EN", "_hpcpro_mips_en", $version);
&zipfile("$builddir\\HPC-SH3-EN", "_hpcpro_sh3_en", $version);
&zipfile("$builddir\\HPC-SH4-EN", "_hpcpro_sh4_en", $version);
&zipfile("$builddir\\PKTPC-ARM-EN", "_pktpc_arm_en", $version);
&zipfile("$builddir\\PKTPC-MIPS-EN", "_pktpc_mips_en", $version);
&zipfile("$builddir\\PKTPC-SH3-EN", "_pktpc_sh3_en", $version);
&zipfile("$builddir\\PSPC-MIPS-EN", "_pspc_mips_en", $version);
&zipfile("$builddir\\PSPC-SH3-EN", "_pspc_sh3_en", $version);
&zipfile("$builddir\\BE300-MIPS-EN", "_be300_mips_en", $version);

&zipfile("$builddir\\HPC-ARM-JP", "_hpcpro_arm_jp", $version);
&zipfile("$builddir\\HPC-MIPS-JP", "_hpcpro_mips_jp", $version);
&zipfile("$builddir\\HPC-SH3-JP", "_hpcpro_sh3_jp", $version);
&zipfile("$builddir\\HPC-SH4-JP", "_hpcpro_sh4_jp", $version);
&zipfile("$builddir\\PKTPC-ARM-JP", "_pktpc_arm_jp", $version);
&zipfile("$builddir\\PKTPC-MIPS-JP", "_pktpc_mips_jp", $version);
&zipfile("$builddir\\PKTPC-SH3-JP", "_pktpc_sh3_jp", $version);
&zipfile("$builddir\\PSPC-MIPS-JP", "_pspc_mips_jp", $version);
&zipfile("$builddir\\PSPC-SH3-JP", "_pspc_sh3_jp", $version);
&zipfile("$builddir\\BE500-MIPS-JP", "_be500_mips_jp", $version);

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
