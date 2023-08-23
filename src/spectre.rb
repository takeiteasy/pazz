require 'ffi'
require 'rbconfig'

def LibraryExtension
    case RbConfig::CONFIG['host_os']
    when /mswin|msys|mingw|cygwin|bccwin|wince|emc/
        ".dll"
    when /darwin|mac os/
        ".dylib"
    when /linux|solaris|bsd/
        ".so"
    else
        raise Exception.new "Unknown OS: #{host_os.inspect}"
    end
end

module Spectre
    extend FFI::Library
    ffi_lib "build/libspectre" + LibraryExtension()
    
    Template = enum(:MAXIMUM, 0,
                    :LONG, 1,
                    :MEDIUM, 2,
                    :SHORT, 3,
                    :BASIC, 4,
                    :PIN, 5,
                    :NAME, 6,
                    :PHRASE, 7)
                    
    attach_function :SpectreGenerate, [:pointer, :pointer, :pointer, :int, :pointer, Template], :pointer
end