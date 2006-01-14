#
# TOMBO Build program for ActivePerl(Win32)

$version = "Tombo_20051228";

##########################################################################
# Tool definition
##########################################################################

use File::Glob ':glob';
use File::Path;
use File::Copy;
#use File::DosGlob 'glob';

$zippg = "\"C:\\Program Files\\mzp\\mzp.exe\"";

##########################################################################
# File definition
##########################################################################

$buildroot = "C:\\work2\\SF\\TOMBO";
$docroot = "C:\\work2\\SF\\Docs";
$builddir  = "C:\\temp\\Tombo-Rel";

$be500root = "E:\\work2\\BE500SDK\\Installer\\Japanese PC Tools";
$be300root = "E:\\work2\\BE300SDK_1012\\Installer\\English PC Tools";

#@docsJP = (
#	["", "$docroot\\Jp\\COPYING-2_0.txt"],
#	["", "$docroot\\Jp\\Readme.txt"],
#	["Docs", "$docroot\\Jp\\Docs\\tombo.css"],
#	["Docs", "$docroot\\Jp\\Docs\\*.html"],
#	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.bmp"],
#	["Docs\\image", "$docroot\\Jp\\Docs\\image\\*.png"],
#	["", "$buildroot\\etc\\vfolder.xml"],
#	["", "$buildroot\\etc\\TOMBO_vfolder.dtd"],
#);
#
#@docsEN = (
#	["", "$docroot\\En\\COPYING-2_0.txt"],
#	["", "$docroot\\En\\Readme.txt"],
#	["Docs", "$docroot\\DocBooks\\UserGuide_en.chm"],
##	["Docs", "$docroot\\En\\Docs\\tombo.css"],
##	["Docs", "$docroot\\En\\Docs\\*.html"],
##	["Docs\\image", "$docroot\\En\\Docs\\image\\*.bmp"],
##	["Docs\\image", "$docroot\\En\\Docs\\image\\*.png"],
#	["", "$buildroot\\etc\\vfolder.xml"],
#	["", "$buildroot\\etc\\TOMBO_vfolder.dtd"],
#);

# for SNAPSHOT
@docsJP = (
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
);
@docsEN = (
	["", "$docroot\\En\\COPYING-2_0.txt"],
);

@bin_hpc_arm_jp = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_hpc_mips_jp = (
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_hpc_sh3_jp = (
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_hpc_sh4_jp =(
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_sig3 = (
	["Bin\\HPCPro\\Sig3", "$buildroot\\HpcPro\\Tombo\\ARMSig3\\Tombo.exe"],
	["Bin\\HPCPro\\Sig3", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_pktpc_arm_jp = (
	["", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\ARM", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\PocketPC\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],

	["", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_jp.arm.CAB"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\PocketPC\\Tombo\\ARMReleaseVGA\\Tombo.exe"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_pktpc_mips_jp =(
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_pktpc_sh3_jp =(
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\Tombo.sh3.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_pspc_mips_jp =(
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\Tombo.mips.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSRel\\Tombo.exe"],
);

@bin_pspc_sh3_jp =(
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\Tombo.sh3.CAB"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3Rel\\Tombo.exe"],
);

@bin_be500 = (
#	["Bin\\BE500", "$be500root\\Setup.exe"],
#	["Bin\\BE500", "$be500root\\Setup.ini"],
#	["Bin\\BE500", "$be500root\\Tombo.cbea"],
	["Bin\\BE500", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\BE500", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_win32_jp = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],
	["", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
);

@bin_hpc_arm_en = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_hpc_mips_en = (
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_hpc_sh3_en = (
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_hpc_sh4_en = (
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);
@bin_pktpc_arm_en = (
	["", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\ARM", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\PocketPC\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],

	["", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_en.arm.CAB"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\PocketPC\\Tombo\\ARMReleaseVGA\\Tombo.exe"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\PocketPC\\ARMVGA", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_pktpc_mips_en = (
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_pktpc_sh3_en = (
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\TomboE.sh3.CAB"],
	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\PocketPC\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_pspc_sh3_en = (
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\CabWiz\\TomboE.mips.CAB"],
	["Bin\\PsPC\\SH3", "$buildroot\\PsPC\\Tombo\\SH3Rel\\Tombo.exe"],
);

@bin_pspc_mips_en = (
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\CabWiz\\TomboE.sh3.CAB"],
	["Bin\\PsPC\\MIPS", "$buildroot\\PsPC\\Tombo\\MIPSRel\\Tombo.exe"],
);

@bin_be300 = (
#	["Bin\\BE300", "$be300root\\Setup.exe"],
#	["Bin\\BE300", "$be300root\\Setup.ini"],
#	["Bin\\BE300", "$be300root\\Tombo.cbea"],
	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\BE300", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

@bin_win32_en = (
	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],
	["", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
);

##########################################################################
# Archive file configuration
##########################################################################

@buildlistCEJP = (
	@bin_hpc_arm_jp, @bin_hpc_mips_jp, @bin_hpc_sh3_jp, @bin_hpc_sh4_jp,
	@bin_pktpc_arm_jp, @bin_pktpc_mips_jp, @bin_pktpc_sh3_jp,
	@bin_pspc_mips_jp, @bin_pspc_sh3_jp,
	@bin_be500, @bin_sig3,

	@docsJP
);

@buildlistWin32JP = ( @bin_win32_jp,  @docsJP);
@buildlist_HPC_JP = ( @bin_hpc_arm_jp, @bin_hpc_mips_jp, @bin_hpc_sh3_jp, @bin_hpc_sh4_jp,  @docsJP);
@buildlist_PKTPC_JP = ( @bin_pktpc_arm_jp, @bin_pktpc_mips_jp, @bin_pktpc_sh3_jp,  @docsJP);
@buildlist_PSPC_JP = ( @bin_pspc_mips_jp, @bin_pspc_sh3_jp,  @docsJP);
@buildlist_BE500_JP = ( @bin_be500, @docsJP);
@buildlist_SIG3 = ( @bin_sig3, @docsJP);

@buildlistCEEN = (
	@bin_hpc_arm_en, @bin_hpc_mips_en, @bin_hpc_sh3_en, @bin_hpc_sh4_en,
	@bin_pktpc_arm_en, @bin_pktpc_mips_en, @bin_pktpc_sh3_en,
	@bin_pspc_sh3_en, @bin_pspc_mips_en,
	@bin_be300,

	@docsEN
);

@buildlistWin32EN = ( @bin_win32_en, @docsEN );
@buildlist_HPC_EN = ( @bin_hpc_arm_en, @bin_hpc_mips_en, @bin_hpc_sh3_en, @bin_hpc_sh4_en, @docsEN );
@buildlist_PKTPC_EN = ( @bin_pktpc_arm_en, @bin_pktpc_mips_en, @bin_pktpc_sh3_en, @docsEN );
@buildlist_PSPC_EN = ( @bin_pspc_sh3_en, @bin_pspc_mips_en, @docsEN );
@buildlist_BE300_EN = ( @bin_be300, @docsEN );


# for each platforms 

@bl_HPC_ARM_JP = (@bin_hpc_arm_jp, @docsJP);
@bl_HPC_MIPS_JP = (@bin_hpc_mips_jp, @docsJP);
@bl_HPC_SH3_JP = (@bin_hpc_sh3_jp, @docsJP);
@bl_HPC_SH4_JP = (@bin_hpc_sh4_jp, @docsJP);
@bl_PKTPC_ARM_JP = (@bin_pktpc_arm_jp, @docsJP);
@bl_PKTPC_MIPS_JP = (@bin_pktpc_mips_jp, @docsJP);
@bl_PKTPC_SH3_JP = (@bin_pktpc_sh3_jp, @docsJP);
@bl_PSPC_MIPS_JP = (@bin_pspc_mips_jp, @docsJP);
@bl_PSPC_SH3_JP = (@bin_pspc_sh3_jp, @docsJP);

@bl_SIG3 = ( @bin_sig3, @docsJP);

@bl_HPC_ARM_EN = (@bin_hpc_arm_en, @docsEN);
@bl_HPC_MIPS_EN = (@bin_hpc_mips_en, @docsEN);
@bl_HPC_SH3_EN = (@bin_hpc_sh3_en, @docsEN);
@bl_HPC_SH4_EN = (@bin_hpc_sh4_en, @docsEN);
@bl_PKTPC_ARM_EN = (@bin_pktpc_arm_en, @docsEN);
@bl_PKTPC_MIPS_EN = (@bin_pktpc_mips_en, @docsEN);
@bl_PKTPC_SH3_EN = (@bin_pktpc_sh3_en, @docsEN);
@bl_PSPC_MIPS_EN = (@bin_pspc_mips_en, @docsEN);
@bl_PSPC_SH3_EN = (@bin_pspc_sh3_en, @docsEN);


@bl_snapshot = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\Tombo\\MIPSRel\\Tombo.exe"],
);

##########################################################################
# Program section
##########################################################################

@archlist = (
#	[\@buildlistCEJP, "CE-JP", "_ce_all_jp"],
#	[\@buildlistCEEN, "CE-EN", "_ce_all_en"],
#	[\@buildlistWin32JP, "Win32-JP", "_desktop_jp"],
#	[\@buildlistWin32EN, "Win32-EN", "_desktop_en"],
##	[\@buildlist_HPC_JP, "HPC-JP", "_hpcpro_jp"],
##	[\@buildlist_PKTPC_JP, "PKTPC-JP", "_pktpc_jp"],
##	[\@buildlist_PSPC_JP, "PSPC-JP", "_pspc_jp"],
##	[\@buildlist_BE500_JP, "BE500-JP", "_be500_jp"],
##	[\@buildlist_HPC_EN, "HPC-EN", "_hpcpro_en"],
##	[\@buildlist_PKTPC_EN, "PKTPC-EN", "_pktpc_en"],
##	[\@buildlist_PSPC_EN, "PSPC-EN", "_pspc_en"],
##	[\@buildlist_BE300_EN, "BE300-EN", "_be300_en"],
#
##	[\@bl_HPC_ARM_JP, "HPC-ARM-JP", "_hpcpro_arm_jp"],
##	[\@bl_HPC_MIPS_JP,"HPC-MIPS-JP", "_hpcpro_mips_jp"],
##	[\@bl_HPC_SH3_JP,"HPC-SH3-JP", "_hpcpro_sh3_jp"],
##	[\@bl_HPC_SH4_JP,"HPC-SH4-JP", "_hpcpro_sh4_jp"],
#	[\@bl_SIG3, "SIG3", "_sig3"],
	[\@bl_PKTPC_ARM_JP,"PKTPC-ARM-JP", "_pktpc_arm_jp"],
#	[\@bl_PKTPC_MIPS_JP,"PKTPC-MIPS-JP", "_pktpc_mips_jp"],
##	[\@bl_PKTPC_SH3_JP,"PKTPC-SH3-JP", "_pktpc_sh3_jp"],
##	[\@bl_PSPC_MIPS_JP,"PSPC-MIPS-JP", "_pspc_mips_jp"],
##	[\@bl_PSPC_SH3_JP,"PSPC-SH3-JP", "_pspc_sh3_jp"],
##	[\@bl_HPC_MIPS_JP,"HPC-MIPS-JP", "_hpcpro_mips_jp"],
#
##	[\@bl_HPC_ARM_EN, "HPC-ARM-EN", "_hpcpro_arm_en"],
##	[\@bl_HPC_MIPS_EN,"HPC-MIPS-EN", "_hpcpro_mips_en"],
##	[\@bl_HPC_SH3_EN,"HPC-SH3-EN", "_hpcpro_sh3_en"],
##	[\@bl_HPC_SH4_EN,"HPC-SH4-EN", "_hpcpro_sh4_en"],
	[\@bl_PKTPC_ARM_EN,"PKTPC-ARM-EN", "_pktpc_arm_en"],
#	[\@bl_PKTPC_MIPS_EN,"PKTPC-MIPS-EN", "_pktpc_mips_en"],
##	[\@bl_PKTPC_SH3_EN,"PKTPC-SH3-EN", "_pktpc_sh3_en"],
##	[\@bl_PSPC_MIPS_EN,"PSPC-MIPS-EN", "_pspc_mips_en"],
##	[\@bl_PSPC_SH3_EN,"PSPC-SH3-EN", "_pspc_sh3_en"],
#
##	[\@bl_snapshot, "SNAPSHOT", "snapshot"],
);

foreach $arch (@archlist) {
	my ($al, $dl, $suffix) = @{$arch};
	&cpfile ("$builddir\\$version\\$dl\\$version", $al);
	&zipfile("$builddir\\$version\\$dl", $suffix, $version);
}

exit 0;

##########################################################################
# Subroutines
##########################################################################

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
