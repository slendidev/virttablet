{ pkgs ? import <nixpkgs> {} }:

let
	kernel = pkgs.linuxKernel.kernels.linux_zen;
in
	pkgs.mkShell {
		name = "kernel-module-dev";

		buildInputs = with pkgs; [
			kernel.dev
			clang-tools
			bear
			nukeReferences
		];

		KERNEL_DIR = "${kernel.dev}/lib/modules/${kernel.modDirVersion}/build";
		MODULE_DIR = "${kernel.dev}/lib/modules/${kernel.modDirVersion}";

		shellHook = ''
			echo "Ready to build kernel modules for kernel ${kernel.modDirVersion}"
		'';
	}
