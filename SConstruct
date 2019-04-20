
binaries = ['zif','zdmp', 'zcrape']
app      = 'Zif'
version  = '0.6.0-Beta'

source  = ['Source/common/ConsoleImpl.cpp']

# Get a build environment
env,libs = SConscript('Platform/build.scons', ['app', 'version'])
env.Append(CCFLAGS = ['-DTERMINAL_EMULATOR'])

# Project specific build config
debug = ARGUMENTS.get('debug', 0)
if int(debug) == 0:
   env.Append(CCFLAGS = ['-O3', '-DNDEBUG'])
else:
   env.Append(CCFLAGS = ['-O0', '-g'])

env.Append(CPPPATH = ['Source'])

# Builders
exe = []
for binary in binaries:
   exe += env.Program(binary, source+['Source/'+binary+'.cpp'])
   Depends(exe, libs)

env.Tar(app+'_'+env['target']+'_'+env['machine']+'_'+version+'.tgz',
        ['zif', env['platform_files'], 'README', 'Games', 'Images', 'Scripts'])

