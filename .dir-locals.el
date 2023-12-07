;;; .dir-locals.el --

;;; Commentary:

;;; Code:
(
 (c-mode .
         ((eval  . (progn

                     (let ((includes-list (list
                                           (substitute-in-file-name "$rcsw/include")
                                           (substitute-in-file-name "$rcsw/ext")
                                           (concat (projectile-project-root) "include")
                                           )
                                          ))
                       (setq flycheck-clang-include-path includes-list)
                       (setq flycheck-gcc-include-path includes-list)
                       (setq cc-search-directories includes-list)
                       )

                     (let ((defs-list (list
                                       "LIBRA_ER=LIBRA_ERL_ALL"
                                       "RCSW_PTR_ALIGN=4"
                                       "RCSW_CONFIG_PTR_ALIGN=4"
                                       )
                             ))
                       (setq flycheck-clang-definitions defs-list)
                       (setq flycheck-gcc-definitions defs-list)
                       )

                       (setq flycheck-clang-args '("-std=gnu11"))
                       (setq flycheck-gcc-args '("-std=gnu11"))
                     )
                 ))
         )
 (nil . ((eval .
               (progn
                 (add-to-list 'projectile-globally-ignored-directories
                              "docs/_build")
                 (add-to-list 'projectile-globally-ignored-directories
                              "docs/_api")
                 )
               ))
      )
)
;;; end of .dir-locals.el
