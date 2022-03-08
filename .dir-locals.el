;;; .dir-locals.el --

;;; Commentary:

;;; Code:
((c-mode .
         ((eval  . (progn

                     (let ((includes-list (list
                                           (substitute-in-file-name "$rcsw/include")
                                           (substitute-in-file-name "$rcsw/ext")
                                           (concat (projectile-project-root) "include")
                                           )))
                       (setq flycheck-clang-include-path includes-list)
                       (setq flycheck-gcc-include-path includes-list)
                       (setq flycheck-clang-args '("-std=gnu99"))
                       (setq flycheck-gcc-args '("-std=gnu99"))
                       (setq cc-search-directories includes-list)
                       )
                     )
                 ))
         ))

;;; end of .dir-locals.el
