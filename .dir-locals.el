;;; .dir-locals.el --

;;; Commentary:

;;; Code:
(
 (nil . ((projectile-project-compilation-cmd . "clibra build")))

 (nil . ((projectile-project-compilation-cmd . "clibra build")

         ;; Must be eval-d/set globally, not set buffer-local, because
         ;; that's what LSP expects.
         (eval . (setq lsp-clients-clangd-executable
                       (substitute-in-file-name "clangd"))
               )
         ))

 (c-ts-mode . (
                 (c-ts-mode-indent-offset . 2)
                 (indent-tabs-mode . nil)
                 (tab-width . 2)

              ;; Must be eval-d/set globally, not set buffer-local,
              ;; because that's what LSP expects.
              (eval . (progn
                        (setq lsp-clients-clangd-args
                              (list "--header-insertion=iwyu"
                                    "--header-insertion-decorators=true"
                                    ;; This required because I open files from
                                    ;; the repo, which is in the symlinked ~/git
                                    ;; directory, but cmake resolves all
                                    ;; symlinks when it generates the
                                    ;; compdb. clangd does EXACT path matching
                                    ;; to find the compdb entry for a file, so
                                    ;; without this it says "can't find file",
                                    ;; falls back to internal defaults.
                                    (substitute-in-file-name "--path-mappings=$rcsw=/opt/$USER/git/thesis/rcsw,/opt/$USER/git/thesis/rcsw=$rcsw")))
                        (setq clang-format-style
                              (concat "file:"
                                      (expand-file-name
                                       (substitute-in-file-name "$libra/dots/.clang-format"))))))
              ))
 )
