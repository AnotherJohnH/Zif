
source  = ['Source/zif.cpp',
           'Source/ZLog.cpp',
           'Source/ZConsole.cpp',
           'Source/ZStream.cpp']

binary  = 'zif'
app     = 'Zif'
version = '0.5.0'

# Get a build environment
env,lib = SConscript('Platform/build.scons', ['app', 'version'])

# Project specific build config
env.Append(CCFLAGS = ['-O3', '-DTERMINAL_EMULATOR'])

# Builders
exe = env.Program(binary, source)
Depends(exe, lib)

env.Tar(app+'_'+env['target']+'_'+env['machine']+'_'+version+'.tgz',
        [exe, env['platform_files'], 'LICENSE', 'zif.cfg', 'Games'])

