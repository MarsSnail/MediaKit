{
  'target_defaults': {
    'conditions': [
      ['OS=="linux"', {
        'defines': [
          '__unix__',
          '_LINUX'
        ],
        'cflags': [
          '-Wall'
        ]
      }],
      ['OS=="win"', {
        'defines': [
          'WIN32'
        ],
        'msvs_configuration_attributes': {
          'CharacterSet': '1'
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'WarningLevel': '4',
            'Detect64BitPortabilityProblems': 'true'
          }
        }
      }],
      ['OS=="mac"', {
        'defines': [
          '__unix__',
          '_MACOS'
        ],
        'cflags': [
          '-Wall'
        ]
      }]
    ],

    'configurations': {
      'Debug': {
        'defines': [
          '_DEBUG'
        ],
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
            'cflags': [
              '-g'
            ]
          }],
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '0',
                'MinimalRebuild': 'true',
                'BasicRuntimeChecks': '3',
                'DebugInformationFormat': '4',
                'RuntimeLibrary': '3'  # /MDd
              },
              'VCLinkerTool': {
                'GenerateDebugInformation': 'true',
                'LinkIncremental': '2'
              }
            }
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES'
            }
          }]
        ]
      },

      'Release': {
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
            'cflags': [
              '-O3'
            ]
          }],
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '2',
                'RuntimeLibrary': '2'  # /MD
              }
            }
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
              'GCC_OPTIMIZATION_LEVEL': '3',

              # -fstrict-aliasing. Mainline gcc enables
              # this at -O2 and above, but Apple gcc does
              # not unless it is specified explicitly.
              'GCC_STRICT_ALIASING': 'YES'
            }
          }]
        ]
      }
    }
  },

  'targets': [
    {
      'target_name': 'mediacore',
      'type': 'shared_library',  # executable, <(library)
      # 'dependencies': [
      # ],
      # 'defines': [
      # ],
      'include_dirs': [
      			'MediaCore',
      			'MediaCore/base',
      			'MediaCore/media',
      			'MediaCore/media/ffmpeg',
      			'MediaCore/network',
      			'MediaCore/network/curl',
      			'MediaCore/platform',
      			'MediaCore/platform/linux',
      			'MediaCore/sound',
      			'MediaCore/sound/sdl',
      			'MediaCore/thirdparty',
      			'MediaCore/thirdparty/glew',
      			'MediaCore/thirdparty/include/GL',
      			'MediaCore/timer'	
      ],
      'sources': [
      			'MediaCore/AVPipeline.h',
				'MediaCore/AVPipeline.cpp',
				'MediaCore/AVPipelineDelegate.h',
				'MediaCore/AVPipelineDelegate.cpp',
				'MediaCore/AVPipelineObserver.h',
				'MediaCore/AVPipelineObserver.cpp',
				'MediaCore/base/CommonDef.h',
				'MediaCore/base/ImageType.cpp',
				'MediaCore/base/ImageType.h',
				'MediaCore/base/SnailConfig.h',
				'MediaCore/base/SnailException.h',
				'MediaCore/media/AudioDecodedFrame.h',
				'MediaCore/media/AudioDecoder.h',
				'MediaCore/media/AudioDecoder.cpp',
				'MediaCore/media/MediaContext.h',
				'MediaCore/media/MediaHandler.h',
				'MediaCore/media/MediaHandler.cpp',
				'MediaCore/media/MediaParser.h',
				'MediaCore/media/MediaParser.cpp',
				'MediaCore/media/MediaParserDelegate.h',
				'MediaCore/media/MediaParserDelegate.cpp',
				'MediaCore/media/PlayControl.h',
				'MediaCore/media/PlayControl.cpp',
				'MediaCore/media/VideoDecodedFrame.h',
				'MediaCore/media/VideoDecoder.h',
				'MediaCore/media/VideoDecoder.cpp',
				'MediaCore/media/ffmpeg/AudioDecoderFFmpeg.h',
				'MediaCore/media/ffmpeg/AudioDecoderFFmpeg.cpp',
				'MediaCore/media/ffmpeg/MediaParserFFmpeg.h',
				'MediaCore/media/ffmpeg/MediaParserFFmpeg.cpp',
				'MediaCore/media/ffmpeg/VideoDecoderFFmpeg.h',
				'MediaCore/media/ffmpeg/VideoDecoderFFmpeg.cpp',
				'MediaCore/network/FileStreamProvider.h',
				'MediaCore/network/FileStreamProvider.cpp',
				'MediaCore/network/InputStreamProvider.h',
				'MediaCore/network/InputStreamProvider.cpp',
				'MediaCore/network/IOChannel.h',
				'MediaCore/network/IOChannel.cpp',
				'MediaCore/network/NetworkAdapter.h',
				'MediaCore/network/Url.h',
				'MediaCore/network/Url.cpp',
				'MediaCore/network/curl/CurlAdapter.cpp',
				'MediaCore/platform/PlatformType.h',
				'MediaCore/platform/SharedTimer.h',
				'MediaCore/platform/ThreadGlobalData.h',
				'MediaCore/platform/ThreadGlobalData.cpp',
				'MediaCore/platform/ThreadTimersManager.h',
				'MediaCore/platform/ThreadTimersManager.cpp',
				'MediaCore/platform/Timer.h',
				'MediaCore/platform/Timer.cpp',
				'MediaCore/platform/linux/SharedTimerLinux.cpp',
				'MediaCore/sound/SoundHandler.h',
				'MediaCore/sound/SoundHandler.cpp',
				'MediaCore/sound/InputStream.h',
				'MediaCore/sound/AuxStream.h',
				'MediaCore/sound/AuxStream.cpp',
				'MediaCore/sound/sdl/SoundHandlerSDL.h',
				'MediaCore/sound/sdl/SoundHandlerSDL.cpp',
				'MediaCore/thirdparty/glew/src/glew.c',
				'MediaCore/thirdparty/glew/include/GL/glew.h',
				'MediaCore/thirdparty/glew/include/GL/glxew.h',
				'MediaCore/thirdparty/glew/include/GL/wglew.h',
				'MediaCore/timer/ClockTime.h',
				'MediaCore/timer/ClockTime.cpp',
				'MediaCore/timer/SnailSleep.h',
				'MediaCore/timer/SystemClock.h',
				'MediaCore/timer/SystemClock.cpp',
				'MediaCore/timer/VirtualClock.h',
				'MediaCore/timer/WallClockTimer.h',
				'MediaCore/timer/WallClockTimer.cpp',
      ],
      # 'conditions': [
      #   ['OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
      #     'ldflags': [
      #       '-'
      #     ]
      #   }],
      #   ['OS=="win"', {
      #     'msvs_settings': {
      #       'VCLinkerTool': {
      #         'AdditionalDependencies': '',
      #         'conditions': [
      #           ['library=="static_library"', {
      #             'AdditionalLibraryDirectories': '$(OutDir)\\lib'
      #           }]
      #         ]
      #       }
      #     }
      #   }],
      #   ['OS=="mac"', {
      #     'link_settings': {
      #       'libraries': [
      #         ''
      #       ]
      #     }
      #   }]
      # ]
    }
  ]
}
