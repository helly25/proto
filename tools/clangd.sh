#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) The helly25/mbo authors (helly25.com)
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -euo pipefail

declare -a CLANGD_LOCATIONS=(
  # Bazelmod 8+
  "bazel-bin/external/toolchains_llvm++llvm+llvm_toolchain_llvm_llvm/bin/clangd"
  "external/toolchains_llvm++llvm+llvm_toolchain_llvm_llvm/bin/clangd"
  # Bazelmod 7+
  "bazel-bin/external/toolchains_llvm~~llvm~llvm_toolchain_llvm_llvm/bin/clangd"
  "external/toolchains_llvm~~llvm~llvm_toolchain_llvm_llvm/bin/clangd"
  # Workspace
  "bazel-bin/external/llvm_toolchain_llvm/bin/clangd"
  "external/llvm_toolchain_llvm/bin/clangd"
  # System
  "$(which clangd-23)"
  "$(which clangd-22)"
  "$(which clangd-21)"
  "$(which clangd-20)"
  "$(which clangd-19)"
  "$(which clangd-18)"
  "$(which clangd-17)"
  "$(which clangd-16)"
  "$(which clangd-15)"
  "$(which clangd)"
)

for CLANGD in "${CLANGD_LOCATIONS[@]}"; do
  if [ -n "${CLANGD}" ] && [ -x "${CLANGD}" ]; then
    "${CLANGD}" "${@}"
    exit 0
  fi
done

die "No clangd was found"
